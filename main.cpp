#include <SDL.h>
#include <stdio.h>
#include "core.h"

#define WIN_W 1366
#define WIN_H 768

static float time; /* global time in secs */
static float dt; /* delta time in secs */
static float2 last_mouse_pos;

float2 get_mouse_pos()
{
	float2 pos;
	int x, y;

	SDL_GetMouseState(&x, &y);
	pos.x = x;
	pos.y = y;
	return pos;
}

float2 get_mouse_dpos()
{
	return sub2f(get_mouse_pos(), last_mouse_pos);
}

static void update_camera_movement(struct camera *p, float deltatime, float2 mouse_dpos)
{
	const Uint8 *state = SDL_GetKeyboardState(NULL);
	Uint32 button_bits = SDL_GetMouseState(NULL, NULL);
	float cam_speed = state[SDL_SCANCODE_LSHIFT] ? 50.0f : 10.0f;
	float mouse_insensity = 0.25f;
	float speed_scale = deltatime * cam_speed;

	mouse_dpos.y *= -1.0f; /* positive if the mouse cursor moves up */

	if (button_bits & SDL_BUTTON_RMASK) {
		p->node.rot = quat_mul(p->node.rot, quat_rotateaxis(float3_up, mouse_dpos.x * mouse_insensity * deltatime));
		p->node.rot = quat_mul(p->node.rot, quat_rotateaxis(node_right(&p->node), -mouse_dpos.y * mouse_insensity * deltatime));
	} else if (button_bits & SDL_BUTTON_LMASK) {
		float3 motion_vector;

		p->node.rot = quat_mul(p->node.rot, quat_rotateaxis(float3_up, mouse_dpos.x * mouse_insensity * deltatime));
		motion_vector = scale3f(node_forward(&p->node), mouse_dpos.y * speed_scale);
		p->node.pos = add3f(p->node.pos, motion_vector);
	} else if (button_bits & SDL_BUTTON_MMASK) {
		float3 motion_vector;

		motion_vector = scale3f(node_right(&p->node), -mouse_dpos.x * speed_scale / 4.0f);
		p->node.pos = add3f(p->node.pos, motion_vector);

		motion_vector = scale3f(node_up(&p->node), -mouse_dpos.y * speed_scale / 4.0f);
		p->node.pos = add3f(p->node.pos, motion_vector);
	}

	if (state[SDL_SCANCODE_A]) {
		float3 motion_vector = scale3f(quat_left(p->node.rot), speed_scale);

		p->node.pos = add3f(p->node.pos, motion_vector);
	} else if (state[SDL_SCANCODE_D]) {
		float3 motion_vector = scale3f(quat_right(p->node.rot), speed_scale);

		p->node.pos = add3f(p->node.pos, motion_vector);
	} else if (state[SDL_SCANCODE_W]) {
		float3 motion_vector = scale3f(quat_forward(p->node.rot), speed_scale);

		p->node.pos = add3f(p->node.pos, motion_vector);
	} else if (state[SDL_SCANCODE_S]) {
		float3 motion_vector = scale3f(quat_backward(p->node.rot), speed_scale);

		p->node.pos = add3f(p->node.pos, motion_vector);
	}
}

/* https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glDebugMessageCallback.xhtml */
static void APIENTRY
gl_message_callback(GLenum source, GLenum type, GLuint id,
		    GLenum severity, GLsizei length,
		    const GLchar *message, const void *userparam)
{
	switch (severity) {
	case GL_DEBUG_SEVERITY_NOTIFICATION: /* trace */
		break;
	case GL_DEBUG_SEVERITY_LOW: /* info */
		printf("%s\n", message);
		break;
	case GL_DEBUG_SEVERITY_MEDIUM: /* warning */
		fprintf(stderr, "%s\n", message);
		break;
	case GL_DEBUG_SEVERITY_HIGH: /* error */
		fprintf(stderr, "%s\n", message);
		break;
	default:
		/* assert(0 && "Unknown severity level."); */
		break;
	}
}

int main(int argc, char *argv[])
{
	SDL_Window *win;
	SDL_GLContext glctx;

	SDL_Init(SDL_INIT_EVERYTHING);
#ifdef _DEBUG
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
#endif
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY); /* don't use VAO */
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);
	win = SDL_CreateWindow("Untitled", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIN_W, WIN_H, SDL_WINDOW_OPENGL);
	glctx = SDL_GL_CreateContext(win);
	gladLoadGLLoader(SDL_GL_GetProcAddress);

	glFrontFace(GL_CW);

	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback(gl_message_callback, NULL);

	gfx_init(WIN_W, WIN_H);
	load_scene("demo_scene.json");

	last_mouse_pos = get_mouse_pos();
	time = SDL_GetTicks() / 1000.0f;
	dt = 0.0f;

	for (;;) {
		SDL_Event event;

		last_mouse_pos = get_mouse_pos();
		dt = SDL_GetTicks() / 1000.0f - time;
		time = time + dt;

		while (SDL_PollEvent(&event) == 1) {
			switch (event.type) {
			case SDL_KEYDOWN:
				if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE)
					goto quit;
				break;
			case SDL_QUIT:
				goto quit;
			}
		}

		update_camera_movement(main_cam, dt, get_mouse_dpos());

		glViewport(0, 0, WIN_W, WIN_H);
		gfx_draw_scene(main_cam, &mesh_ent_list, &light_list);
		SDL_GL_SwapWindow(win);
	}

quit:
	SDL_GL_DeleteContext(glctx);
	SDL_DestroyWindow(win);
	SDL_Quit();
	return 0;
}
