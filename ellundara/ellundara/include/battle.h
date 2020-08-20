#pragma once
#include <onions/matrix.h>
#include "state.h"

#define GRID_TILE_SIZE 128
#define GRID_TILE_HEIGHT (GRID_TILE_SIZE * 9 / 32)


struct BattleTileType
{
	// The sprite for the top of the tile.
	Sprite* top;

	// The sprite for the side of the tile.
	Sprite* side;
};

class BattleTileSet
{
private:
	static std::unordered_map<std::string, BattleTileSet*> m_Sets;

	// The sprite sheet.
	SpriteSheet* m_SpriteSheet;

	std::unordered_map<std::string, BattleTileType*> m_Types;

	BattleTileSet(std::string id);

public:
	static BattleTileSet* get_tile_set(std::string id);

	SpriteSheet* get_sprite_sheet();

	const BattleTileType* get_tile_type(std::string type) const;
};


class BattleObject;
class BattleTerrain;

struct BattleTile
{
	// The type of tile.
	const BattleTileType* type;

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
	// The tile set used for the grid.
	BattleTileSet* m_TileSet;

	// The array of tiles.
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

	/// <summary>Displays the grid.</summary>
	/// <param name="dx">The x-direction from back to front.</param>
	/// <param name="dy">The y-direction from back to front.</param>
	void display(int dx, int dy, Palette* palette) const;
};

// The state when the game is in a battle.
class BattleState : public State, public UpdateListener
{
protected:
	// The current angle that the grid is being viewed from.
	static float m_Angle;

	// The grid for the battle.
	BattleGrid m_Grid;

	// The orthographic transform matrix.
	mat4x4f m_Transform;

	// The current camera position.
	vec2f m_Camera;

	// The current zoom factor.
	float m_Zoom;

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

	static float get_angle();
};



/*
	OBJECTS
*/

class BattleObject
{
protected:
	static bool m_IsObjectDataLoaded;

	static std::unordered_map<std::string, std::unordered_map<std::string, std::string>*> m_ObjectData;

	static std::unordered_map<std::string, BattleObject*> m_Objects;

	BattleObject(std::string id);

public:
	static BattleObject* get_object(std::string id);

	virtual void display() const = 0;
};

class BillboardedBattleObject : public BattleObject
{
protected:
	SpriteGraphic* m_Sprite;

public:
	BillboardedBattleObject(std::string id, SpriteGraphic* sprite);

	void display() const;
};

class StaticBattleObject : public BillboardedBattleObject
{
public:
	StaticBattleObject(std::string id, std::string sprite_sheet, std::string sprite);
};

class BattleActor : public BattleObject
{

};



/*
	TERRAIN
*/

class BattleTerrain
{
public:
	/// <summary>Retrieves the height displacement of an object on top of the terrain.</summary>
	/// <returns>How far an object on or in the terrain is displaced from the ground.</returns>
	virtual int get_height() = 0;

	virtual void display() const = 0;
};