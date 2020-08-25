#include "../include/controls.h"
#include "../include/state.h"
#include "../include/battle.h"

State* g_State = nullptr;

State* get_state()
{
	return g_State;
}

void set_state(State* state)
{
	if (g_State)
	{
		delete g_State;
	}

	Application* app = get_application_settings();
	g_State = state;
	g_State->set_bounds(0, 0, app->width, app->height);
}


void display()
{
	if (g_State)
		g_State->display();
}

int main()
{
	// Initialize the Onion library.
	onion_init("settings.ini");

	// Register controls.
	register_keyboard_control(CONTROL_SELECT, CONTROL_SELECT_DEFAULT);
	register_keyboard_control(CONTROL_CANCEL, CONTROL_CANCEL_DEFAULT);

	register_keyboard_control(CONTROL_MOVE_LEFT, CONTROL_MOVE_LEFT_DEFAULT);
	register_keyboard_control(CONTROL_MOVE_RIGHT, CONTROL_MOVE_RIGHT_DEFAULT);
	register_keyboard_control(CONTROL_MOVE_DOWN, CONTROL_MOVE_DOWN_DEFAULT);
	register_keyboard_control(CONTROL_MOVE_UP, CONTROL_MOVE_UP_DEFAULT);

	register_keyboard_control(CONTROL_ROTATE_LEFT, CONTROL_ROTATE_LEFT_DEFAULT);
	register_keyboard_control(CONTROL_ROTATE_RIGHT, CONTROL_ROTATE_RIGHT_DEFAULT);

	// Initialize the global state.
	set_state(new BattleState("debug"));

	// Run the main loop, using the above function to display.
	onion_main(&display);
}