#pragma once
#include <onions/matrix.h>
#include "state.h"

#define GRID_TILE_SIZE 128


class BattleObject;
class BattleTerrain;

struct BattleTile
{
	static SpriteSheet* sprite_sheet;

	// The sprite for the top of the tile.
	SPRITE_KEY top;

	// The sprite for the sides of the tile.
	SPRITE_KEY side;

	// The height of the tile.
	int height;

	// What, if anything, is located on the tile.
	BattleObject* obj;

	// If the tile has any special terrain.
	BattleTerrain* terrain;
};

class BattleGrid
{
protected:
	BattleTile* m_Tiles;

public:
	// The width of the grid.
	int width;

	// The height of the grid.
	int height;

	/// <summary>Loads the battle grid from a map file.</summary>
	/// <param name="map">The ID of the battle map.</param>
	BattleGrid(std::string map);

	/// <summary>Retrieves the grid tile at the given coordinates.</summary>
	/// <param name="x">The x-coordinate of the grid tile.</param>
	/// <param name="y">The y-coordinate of the grid tile.</param>
	/// <returns>A pointer to the tile.</returns>
	BattleTile* get_tile(int x, int y);

	/// <summary>Retrieves the grid tile at the given coordinates.</summary>
	/// <param name="x">The x-coordinate of the grid tile.</param>
	/// <param name="y">The y-coordinate of the grid tile.</param>
	/// <returns>A const pointer to the tile.</returns>
	const BattleTile* get_tile(int x, int y) const;
};

// The state when the game is in a battle.
class BattleState : public State, public UpdateListener
{
protected:
	// The grid for the battle.
	BattleGrid m_Grid;

	// The orthographic transform matrix.
	mat4x4f m_Transform;

	// The current camera position.
	vec2f m_Camera;

	// The current zoom factor.
	float m_Zoom;

	// The current angle that the grid is being viewed from.
	float m_Angle;

	/// <summary>Adjusts the transform in response to the bounds changing.</summary>
	void __set_bounds();

	/// <summary>Displays the state.</summary>
	void __display() const;

	/// <summary>Updates the state by a time step.</summary>
	/// <param name="frames_passed">The number of frames that have passed since the last update.</param>
	void __update(int frames_passed);

public:
	/// <summary>Initializes a new battle state.</summary>
	/// <param name="map">The ID of the battle map.</param>
	BattleState(std::string map);
};