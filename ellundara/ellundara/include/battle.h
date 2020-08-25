#pragma once
#include <unordered_set>
#include <onions/matrix.h>
#include "state.h"

#define GRID_TILE_SIZE 128
#define GRID_TILE_HEIGHT (GRID_TILE_SIZE * 9 / 32)


namespace Battle
{

	class Highlight;


	struct TileType
	{
		// The sprite for the top of the tile.
		Sprite* top;

		// The sprite for the side of the tile.
		Sprite* side;
	};

	class TileSet
	{
	private:
		static std::unordered_map<std::string, TileSet*> m_Sets;

		// The sprite sheet.
		SpriteSheet* m_SpriteSheet;

		std::unordered_map<std::string, TileType*> m_Types;

		TileSet(std::string id);

	public:
		static TileSet* get_tile_set(std::string id);

		SpriteSheet* get_sprite_sheet();

		const TileType* get_tile_type(std::string type) const;
	};


	class Object;
	class Terrain;

	struct Tile
	{
		// The type of tile.
		const TileType* type;

		// The height of the tile.
		int height;

		// What, if anything, is located on the tile.
		Object* obj;

		// If the tile has any special terrain.
		Terrain* terrain;
	};

	class Grid
	{
	protected:
		// The tile set used for the grid.
		TileSet* m_TileSet;

		// The array of tiles.
		Tile* m_Tiles;

	public:
		// The width of the grid.
		int width;

		// The height of the grid.
		int height;

		/// <summary>Loads the battle grid from a map file.</summary>
		/// <param name="map">The ID of the battle map.</param>
		Grid(std::string map);

		/// <summary>Retrieves the grid tile at the given coordinates.</summary>
		/// <param name="x">The x-coordinate of the grid tile.</param>
		/// <param name="y">The y-coordinate of the grid tile.</param>
		/// <returns>A pointer to the tile.</returns>
		Tile* get_tile(int x, int y);

		/// <summary>Retrieves the grid tile at the given coordinates.</summary>
		/// <param name="x">The x-coordinate of the grid tile.</param>
		/// <param name="y">The y-coordinate of the grid tile.</param>
		/// <returns>A const pointer to the tile.</returns>
		const Tile* get_tile(int x, int y) const;

		const SpriteSheet* get_tile_sprite_sheet() const;
	};





	/*
		GRID VIEW
	*/
	
	class Highlight
	{
	public:
		
	};


	class Visibility
	{
	protected:
		// The bounds of what is visible.
		const mat2x2i& m_Bounds;

		// A pointer to the grid.
		Grid* m_Grid;


		// The transform matrix for the grid.
		mat4x4f m_Transform;


		// The current angle that the grid is being viewed from. Uses radians.
		static float m_Angle;

		// The current camera position.
		static vec3f m_Camera;

		// The current zoom factor.
		static float m_Zoom;


		// The angle to set the angle to. Uses radians.
		float m_TargetAngle;

		// The camera position to set the camera to.
		vec3f m_TargetCamera;

		// The zoom factor to set the zoom to.
		float m_TargetZoom;



		// Data structure about a tile that needs to be drawn.
		struct VisibleTile
		{
			// The information of the tile.
			Tile* tile;

			// The x, y, z translation from the previous tile.
			vec3f trans;

			// The heights to draw the horizontal and vertical sides.
			vec2i sides;
		};

		// An array of the visible tiles, in the order that they need to be displayed.
		std::vector<VisibleTile> m_VisibleTiles;


		// The sprite sheet used to draw tiles.
		const SpriteSheet* m_TileSpriteSheet;
		
		// The palette used to display the grid.
		const Palette* m_Palette;


		
		class Highlight
		{
		public:
			/// <summary>Check whether the tile should be highlighted.</summary>
			/// <param name="vis">The grid visualizer.</param>
			/// <param name="tile">A pointer to the tile.</param>
			/// <returns>Whether the indicated tile should be highlighted.</returns>
			virtual bool highlight_tile(const Visibility* vis, Tile* tile) const = 0;

			/// <summary>Displays the highlighting.</summary>
			virtual void display() const = 0;
		};
		
		class Selector : public Highlight
		{
		protected:
			friend class Visibility;

			static Graphic* m_Graphic;

			vec2i m_Tile;

		public:
			Selector();

			void set_tile(Visibility* vis, int x, int y);

			void adjust_tile(Visibility* vis, int dx, int dy);

			bool highlight_tile(const Visibility* vis, Tile* tile) const;

			void display() const;

		} m_Selector;


		/// <summary>Resets the transform matrix.</summary>
		void reset_transform();
		
		/// <summary>Resets which tiles are visible.</summary>
		void reset_visible_tiles();


		/// <summary>Adjusts the angle that the grid is being viewed from.</summary>
		/// <param name="adjustment">The angular adjustment, in radians.</param>
		void adjust_angle(float adjustment);

		/// <summary>Adjusts the camera position.</summary>
		/// <param name="adjustment">The position adjustment.</param>
		void adjust_camera(vec3f adjustment);


		/// <summary>Sets the target camera position.</summary>
		/// <param name="target">The target for the camera position.</param>
		void set_camera_target(vec3f target);


		/// <summary>Displays the base of a tile.</summary>
		/// <param name="vtile">The data for a visible tile.</param>
		void display_tile(const VisibleTile& vtile) const;

		/// <summary>Displays the object on a tile.</summary>
		/// <param name="vtile">The data for a visible tile.</param>
		void display_object(const VisibleTile& vtile) const;

		/// <summary>Displays the terrain of a tile.</summary>
		/// <param name="vtile">The data for a visible tile.</param>
		void display_terrain(const VisibleTile& vtile) const;

	public:
		/// <summary>Retrieves the view angle for the grid.</summary>
		/// <returns>The angle that the grid is being viewed from, in radians.</returns>
		static float get_angle();

		/// <summary>Constructs the view for the grid.</summary>
		/// <param name="bounds">The bounds of the screen.</param>
		/// <param name="grid">The battle grid.</param>
		Visibility(const mat2x2i& bounds, Grid* grid);

		/// <summary>Resets what is visible.</summary>
		void reset();

		/// <summary>Rotates the grid clockwise.</summary>
		void rotate_left();

		/// <summary>Rotates the grid counter-clockwise.</summary>
		void rotate_right();

		/// <summary>Sets which tile is selected.</summary>
		/// <param name="dx">The x-coordinate of the selected tile.</param>
		/// <param name="dy">The y-coordinate of the selected tile.</param>
		void set_selected_tile(int x, int y);

		/// <summary>Adjusts which tile is selected.</summary>
		/// <param name="dx">The change in x-coordinate, before factoring in the camera.</param>
		/// <param name="dy">The change in y-coordinate, before factoring in the camera.</param>
		void adjust_selected_tile(int dx, int dy);

		/// <summary>Updates the view of the grid.</summary>
		/// <param name="frames_passed">The number of frames that passed since the last update.</param>
		void update(int frames_passed);

		/// <summary>Displays the grid.</summary>
		void display() const;
	};



	/*
		OBJECTS
	*/

	class Object
	{
	protected:
		// Whether the data for all objects has been loaded.
		static bool m_IsObjectDataLoaded;

		// A map from an ID to the data for the object.
		static std::unordered_map<std::string, std::unordered_map<std::string, std::string>*> m_ObjectData;

		// A map from an ID to the loaded object.
		static std::unordered_map<std::string, Object*> m_Objects;

		/// <summary>Adds the object to the map.</summary>
		/// <param name="id">The ID for the object.</param>
		Object(std::string id);

	public:
		/// <summary>Retrieves the object with the given ID.</summary>
		/// <param name="id">The ID of the object to retrieve.</param>
		/// <returns>The object with the given ID.</returns>
		static Object* get_object(std::string id);

		/// <summary>Displays the object.</summary>
		virtual void display() const = 0;
	};

	class BillboardedObject : public Object
	{
	protected:
		SpriteGraphic* m_Sprite;

	public:
		BillboardedObject(std::string id, SpriteGraphic* sprite);

		virtual void display() const;
	};

	class StaticObject : public BillboardedObject
	{
	public:
		StaticObject(std::string id, std::string sprite_sheet, std::string sprite);
	};

	class Actor : public Object
	{

	};



	/*
		TERRAIN
	*/

	class Terrain
	{
	public:
		/// <summary>Retrieves the height displacement of an object on top of the terrain.</summary>
		/// <returns>How far an object on or in the terrain is displaced from the ground.</returns>
		virtual int get_height() = 0;

		/// <summary>Displays the terrain.</summary>
		virtual void display() const = 0;
	};




	/*
		EVENTS
	*/
	
	class Event
	{
	public:
		virtual int start();

		virtual int update();
	};
	
	class Queue
	{
	protected:
		struct EventPriority
		{
			Event* event;
			int priority;
		};

		std::list<EventPriority> m_Queue;

		Event* m_Current = nullptr;

	public:
		void push(Event* event, int priority);

		Event* pop();

		void update();
	};



}




// The state when the game is in a battle.
class BattleState : public State, public UpdateListener, public KeyboardListener
{
protected:
	// The orthographic transform matrix.
	mat4x4f m_Transform;

	// Controls the visibility of the grid.
	Battle::Visibility m_Visibility;


	// The grid for the battle.
	Battle::Grid m_Grid;

	// The queue for battle events and animations and whatever.
	Battle::Queue m_Queue;


	// The thing that shows which tiles are targetable.
	Graphic* m_Targetable;


	// The current phase of battle. True if Player Phase, false if Enemy Phase.
	bool m_Phase;


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

	/// <summary>Prevents the state from registering updates and inputs.</summary>
	void freeze();

	/// <summary>Makes the state start registering updates and inputs.</summary>
	void unfreeze();

	/// <summary>Responds to a keyboard control being pressed.</summary>
	/// <param name="event_data">The data for the event.</param>
	int trigger(const KeyEvent& event_data);
};