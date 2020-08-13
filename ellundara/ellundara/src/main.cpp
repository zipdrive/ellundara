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

	// Initialize the global state.
	set_state(new BattleState("debug"));

	// Run the main loop, using the above function to display.
	onion_main(&display);
}