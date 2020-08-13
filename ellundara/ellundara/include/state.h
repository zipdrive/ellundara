#pragma once
#include <onion.h>

// A state that the game can be in.
class State : public Frame {};

/// <summary>Retrieves the current global state.</summary>
/// <returns>The current global state.</returns>
State* get_state();

/// <summary>Sets the global state.</summary>
/// <param name="state">The new global state.</param>
void set_state(State* state);
