#include <algorithm>
#include <regex>
#include "../../include/controls.h"
#include "../../include/battle.h"

#include <iostream>

#define GRID_COORDINATE(x, y, width) ((x) + ((width) * (y)))

using namespace std;
using namespace Battle;


std::unordered_map<string, TileSet*> TileSet::m_Sets{};

TileSet::TileSet(string id)
{
	string path = "tiles/" + id + ".png";
	m_SpriteSheet = SpriteSheet::generate(path.c_str());

	path = "res/img/tiles/" + id + ".meta";
	LoadFile file(path);

	regex top_regex("(.*)\\s+top");
	regex side_regex("(.*)\\s+side");

	while (file.good())
	{
		unordered_map<string, int> data;
		string line = file.load_data(data);

		smatch match;
		if (regex_match(line, match, top_regex))
		{
			auto iter = m_Types.find(match[1].str());
			if (iter != m_Types.end())
			{
				iter->second->top = Sprite::get_sprite(line);
			}
			else
			{
				TileType* type = new TileType();
				type->top = Sprite::get_sprite(line);
				m_Types.emplace(match[1].str(), type);
			}
		}
		else if (regex_match(line, match, side_regex))
		{
			auto iter = m_Types.find(match[1].str());
			if (iter != m_Types.end())
			{
				iter->second->side = Sprite::get_sprite(line);
			}
			else
			{
				TileType* type = new TileType();
				type->side = Sprite::get_sprite(line);
				m_Types.emplace(match[1].str(), type);
			}
		}
	}
}

TileSet* TileSet::get_tile_set(string id)
{
	auto iter = m_Sets.find(id);
	if (iter != m_Sets.end())
		return iter->second;
	
	TileSet* s = new TileSet(id);
	m_Sets.emplace(id, s);
	return s;
}

SpriteSheet* TileSet::get_sprite_sheet()
{
	return m_SpriteSheet;
}

const TileType* TileSet::get_tile_type(string type) const
{
	auto iter = m_Types.find(type);
	if (iter != m_Types.end())
		return iter->second;
	return nullptr;
}



Grid::Grid(string map)
{
	width = 0;
	height = 0;
	m_Tiles = nullptr;

	LoadFile file("res/maps/" + map + ".txt");

	regex tileset_regex("tileset\\s+(.*)");
	regex tile_regex("tile\\s+(.*)");
	regex obj_regex("obj\\s+(.*)");

	while (file.good())
	{
		// Read the next line of the file
		std::unordered_map<string, int> line;
		string id = file.load_data(line);

		smatch match;
		
		// Load tileset
		if (regex_match(id, match, tileset_regex))
		{
			m_TileSet = TileSet::get_tile_set(match[1].str());
		}

		// Construct tiles
		else if (regex_match(id, match, tile_regex))
		{
			int x = line["x"];
			int y = line["y"];
			int dx = line["dx"];
			int dy = line["dy"];

			if (x + dx > width || y + dy > height)
			{
				int new_width = max(x + dx, width);
				int new_height = max(y + dy, height);

				Tile* new_tiles = new Tile[new_width * new_height];
				for (int k = (new_width * new_height) - 1; k >= 0; --k)
				{
					new_tiles[k].height = 0;
					new_tiles[k].terrain = nullptr;
					new_tiles[k].obj = nullptr;
				}

				for (int i = 0; i < width; ++i)
				{
					for (int j = 0; j < height; ++j)
					{
						new_tiles[GRID_COORDINATE(i, j, new_width)] = m_Tiles[GRID_COORDINATE(i, j, width)];
					}
				}

				if (m_Tiles)
					delete m_Tiles;
				m_Tiles = new_tiles;
				width = new_width;
				height = new_height;
			}

			int h = line["height"];
			const TileType* type = m_TileSet->get_tile_type(match[1].str());

			for (int i = x; i < x + dx; ++i)
			{
				for (int j = y; j < y + dy; ++j)
				{
					Tile* tile = get_tile(i, j);
					tile->type = type;
					tile->height = h;
				}
			}
		}

		// Construct objects
		else if (regex_match(id, match, obj_regex))
		{
			int x = line["x"];
			int y = line["y"];

			Tile* tile = get_tile(x, y);
			if (tile)
				tile->obj = Object::get_object(match[1].str());

			cout << "Object loaded.";
		}
	}
}

Tile* Grid::get_tile(int x, int y)
{
	if (x >= 0 && x < width && y >= 0 && y < height)
		return m_Tiles + GRID_COORDINATE(x, y, width);
	return nullptr;
}

const Tile* Grid::get_tile(int x, int y) const
{
	if (x >= 0 && x < width && y >= 0 && y < height)
		return m_Tiles + GRID_COORDINATE(x, y, width);
	return nullptr;
}

const SpriteSheet* Grid::get_tile_sprite_sheet() const
{
	return m_TileSet->get_sprite_sheet();
}





Graphic* Visibility::Selector::m_Graphic{ nullptr };

Visibility::Selector::Selector()
{
	if (!m_Graphic)
		m_Graphic = SolidColorGraphic::generate(255, 255, 0, 128, GRID_TILE_SIZE, GRID_TILE_SIZE);

	m_Tile = vec2i(0, 0);
}

void Visibility::Selector::set_tile(Visibility* vis, int x, int y)
{
	m_Tile = vec2i(x, y);

	if (m_Tile.get(0) < 0)								m_Tile.set(0, 0, 0);
	else if (m_Tile.get(0) >= vis->m_Grid->width)		m_Tile.set(0, 0, vis->m_Grid->width - 1);

	if (m_Tile.get(1) < 0)								m_Tile.set(1, 0, 0);
	else if (m_Tile.get(1) >= vis->m_Grid->height)		m_Tile.set(1, 0, vis->m_Grid->height - 1);
}

void Visibility::Selector::adjust_tile(Visibility* vis, int dx, int dy)
{
	m_Tile += vec2i(dx, dy);

	if (m_Tile.get(0) < 0)								m_Tile.set(0, 0, 0);
	else if (m_Tile.get(0) >= vis->m_Grid->width)		m_Tile.set(0, 0, vis->m_Grid->width - 1);

	if (m_Tile.get(1) < 0)								m_Tile.set(1, 0, 0);
	else if (m_Tile.get(1) >= vis->m_Grid->height)		m_Tile.set(1, 0, vis->m_Grid->height - 1);
}

bool Visibility::Selector::highlight_tile(const Visibility* vis, Tile* tile) const
{
	return tile == vis->m_Grid->get_tile(m_Tile.get(0), m_Tile.get(1));
}

void Visibility::Selector::display() const
{
	m_Graphic->display();
}



#define ROTATE_SPEED		0.01309f
#define CAMERA_SPEED		0.015f
#define CAMERA_THRESHOLD	1.f

#define QUARTER_PI			0.7853981634f
#define HALF_PI				1.570796327f
#define THREE_QUARTERS_PI	2.35619449f
#define PI					3.1415926535f
#define THREE_HALVES_PI		4.71238898f
#define TWO_PI				6.283185307f

#define TOP_DOWN_ANGLE		0.872664626f

float Visibility::m_Angle{ QUARTER_PI };
vec3f Visibility::m_Camera{};
float Visibility::m_Zoom{ 1.f };

float Visibility::get_angle()
{
	return m_Angle;
}

Visibility::Visibility(const mat2x2i& bounds, Grid* grid) : m_Bounds(bounds)
{
	m_Grid = grid;

	m_Palette = new SinglePalette(vec4f(1.f, 0.f, 0.f, 0.f), vec4f(0.f, 1.f, 0.f, 0.f), vec4f(0.f, 0.f, 1.f, 0.f));

	m_TargetAngle = m_Angle;
}

void Visibility::reset_transform()
{
	// Create the transform matrix
	float rcos = m_Zoom * cosf(TOP_DOWN_ANGLE);
	float rsin = sinf(TOP_DOWN_ANGLE);
	float rcot = cosf(TOP_DOWN_ANGLE) / rsin;
	float bcos = cosf(m_Angle);
	float bsin = sinf(m_Angle);

	float ty = -((bsin * m_Camera.get(0)) + (bcos * m_Camera.get(1)));

	m_Transform.set(0, 0, m_Zoom * bcos);
	m_Transform.set(0, 1, -m_Zoom * bsin);
	m_Transform.set(0, 3, -m_Zoom * ((bcos * m_Camera.get(0)) - (bsin * m_Camera.get(1))));

	m_Transform.set(1, 0, rcos * bsin);
	m_Transform.set(1, 1, rcos * bcos);
	m_Transform.set(1, 2, m_Zoom);
	m_Transform.set(1, 3, (rcos * ty) - (m_Zoom * m_Camera.get(2)));

	m_Transform.set(2, 0, rsin * bsin);
	m_Transform.set(2, 1, rsin * bcos);
	m_Transform.set(2, 2, -rcot);
	m_Transform.set(2, 3, (rsin * ty) + (rcot * m_Camera.get(2)));
}

void Visibility::reset_visible_tiles()
{
	m_VisibleTiles.clear();

	// Decide which order to draw the tiles in
	int dx = sin(m_Angle) > 0 ? -1 : 1;
	int dy = cos(m_Angle) > 0 ? -1 : 1;
	int istart, iend, jstart, jend;
	if (dx < 0)
	{
		istart = m_Grid->width - 1;
		iend = -1;
	}
	else
	{
		istart = 0;
		iend = m_Grid->width;
	}
	if (dy < 0)
	{
		jstart = m_Grid->height - 1;
		jend = -1;
	}
	else
	{
		jstart = 0;
		jend = m_Grid->height;
	}

	// Determine which tiles are visible
	vec2f trans(GRID_TILE_SIZE * istart, GRID_TILE_SIZE * jstart);
	int prev_height = 0;
	for (int i = istart; i != iend; i += dx)
	{
		for (int j = jstart; j != jend; j += dy)
		{
			if (true) // TODO check if visible
			{
				if (Tile* tile = m_Grid->get_tile(i, j))
				{
					const Tile* tx = m_Grid->get_tile(i + dx, j);
					const Tile* ty = m_Grid->get_tile(i, j + dy);

					m_VisibleTiles.push_back({
						tile,
						vec3f(trans, GRID_TILE_HEIGHT * (tile->height - prev_height)),
						vec2i(tile->height - (tx ? tx->height : 0), tile->height - (ty ? ty->height : 0))
					});

					trans = vec2f();
					prev_height = tile->height;
				}
			}

			trans.set(1, 0, trans.get(1) + (dy * GRID_TILE_SIZE));
		}

		trans.set(0, 0, trans.get(0) + (dx * GRID_TILE_SIZE));
		trans.set(1, 0, trans.get(1) + ((jstart - jend) * GRID_TILE_SIZE));
	}
}

void Visibility::reset()
{
	m_TileSpriteSheet = m_Grid->get_tile_sprite_sheet();

	// Reset the transform matrix
	reset_transform();

	// Reset which tiles are visible
	reset_visible_tiles();
}

void Visibility::adjust_angle(float adjustment)
{
	bool dx = sin(m_Angle) > 0.f;
	bool dy = cos(m_Angle) > 0.f;

	m_Angle += adjustment;

	// Clamp the angle and target angle to the range (-PI, PI)
	if (m_Angle > PI)
	{
		m_Angle -= TWO_PI;
		m_TargetAngle -= TWO_PI;
	}
	else if (m_Angle < -PI)
	{
		m_Angle += TWO_PI;
		m_TargetAngle += TWO_PI;
	}

	// TODO make this more efficient?
	reset_transform();

	// If the direction that tiles are drawn in changes, reset which tiles are visible
	if (dx != (sin(m_Angle) > 0.f) || dy != (cos(m_Angle) > 0.f))
	{
		reset_visible_tiles();
	}
	else
	{
		// TODO
	}
}

void Visibility::adjust_camera(vec3f adjustment)
{
	m_Camera += adjustment;

	// TODO make this more efficient
	reset();
}

void Visibility::rotate_left()
{
	if (m_Angle >= m_TargetAngle)
		m_TargetAngle += HALF_PI;
}

void Visibility::rotate_right()
{
	if (m_Angle <= m_TargetAngle)
		m_TargetAngle -= HALF_PI;
}

void Visibility::set_selected_tile(int x, int y)
{
	m_Selector.set_tile(this, x, y);
	m_TargetCamera = vec3f(
		(m_Selector.m_Tile.get(0) + 0.5f) * GRID_TILE_SIZE,
		(m_Selector.m_Tile.get(1) + 0.5f) * GRID_TILE_SIZE,
		m_Grid->get_tile(m_Selector.m_Tile.get(0), m_Selector.m_Tile.get(1))->height * GRID_TILE_HEIGHT
	);
}

void Visibility::adjust_selected_tile(int dx, int dy)
{
	float theta = m_Angle + QUARTER_PI;
	mat2x2i rot(
		round(cos(theta)), round(sin(theta)),
		round(-sin(theta)), round(cos(theta))
	);
	matrix<int, 2, 1> d = rot * vec2i(dx, dy);

	m_Selector.adjust_tile(this, d.get(0), d.get(1));
	m_TargetCamera = vec3f(
		(m_Selector.m_Tile.get(0) + 0.5f) * GRID_TILE_SIZE, 
		(m_Selector.m_Tile.get(1) + 0.5f) * GRID_TILE_SIZE, 
		m_Grid->get_tile(m_Selector.m_Tile.get(0), m_Selector.m_Tile.get(1))->height * GRID_TILE_HEIGHT
	);
}

void Visibility::update(int frames_passed)
{
	if (m_TargetAngle < m_Angle)
	{
		adjust_angle(-frames_passed * ROTATE_SPEED);

		if (m_Angle < m_TargetAngle)
		{
			m_Angle = m_TargetAngle;

			// Reset the transform matrix
			reset();
		}
	}
	else if (m_TargetAngle > m_Angle)
	{
		adjust_angle(frames_passed * ROTATE_SPEED);

		if (m_Angle > m_TargetAngle)
		{
			m_Angle = m_TargetAngle;

			// Reset the transform matrix
			reset();
		}
	}

	if (m_TargetCamera != m_Camera)
	{
		vec3f diff = m_TargetCamera - m_Camera;
		float dist = pow(diff.get(0), 2) + pow(diff.get(1), 2) + pow(diff.get(2), 2);

		if (dist < CAMERA_THRESHOLD)
		{
			m_Camera = m_TargetCamera;
		}
		else
		{
			adjust_camera(frames_passed * CAMERA_SPEED * (m_TargetCamera - m_Camera));
		}
	}
}

void Visibility::display_tile(const VisibleTile& vtile) const
{
	// Draw the ground of the tile
	mat_translate(vtile.trans.get(0), vtile.trans.get(1), vtile.trans.get(2));
	m_TileSpriteSheet->display(vtile.tile->type->top->key, m_Palette);

	// Draw the sides of the tile
	int dh = vtile.sides.get(0);
	if (dh > 0)
	{
		mat_push();

		if (m_Angle < 0.f)//(dx > 0)
		{
			mat_translate(GRID_TILE_SIZE, GRID_TILE_SIZE, 0.f);
			mat_scale(1.f, -1.f, 1.f);
		}

		mat_rotatez(1.5708f);
		mat_rotatex(1.5708f);

		for (int k = 0; k < dh; ++k)
		{
			mat_translate(0.f, -GRID_TILE_HEIGHT, 0.f);
			m_TileSpriteSheet->display(vtile.tile->type->side->key, m_Palette);
		}

		mat_pop();
	}

	dh = vtile.sides.get(1);
	if (dh > 0)
	{
		mat_push();

		if (m_Angle > HALF_PI || m_Angle < -HALF_PI)//(dy > 0)
		{
			mat_translate(GRID_TILE_SIZE, GRID_TILE_SIZE, 0.f);
			mat_scale(-1.f, 1.f, 1.f);
		}

		mat_rotatex(1.5708f);

		for (int k = 0; k < dh; ++k)
		{
			mat_translate(0.f, -GRID_TILE_HEIGHT, 0.f);
			m_TileSpriteSheet->display(vtile.tile->type->side->key, m_Palette);
		}

		mat_pop();
	}
}

void Visibility::display_object(const VisibleTile& vtile) const
{
	mat_translate(vtile.trans.get(0), vtile.trans.get(1), vtile.trans.get(2));

	float theight = ((vtile.tile->terrain ? vtile.tile->terrain->get_height() : 0) * GRID_TILE_HEIGHT) + 0.01f;

	if (vtile.tile->obj)
	{
		mat_translate(0.f, 0.f, theight + 0.01f);
		vtile.tile->obj->display();

		if (m_Selector.highlight_tile(this, vtile.tile))
		{
			mat_translate(0.f, 0.f, -0.01f);
			m_Selector.display();
			mat_translate(0.f, 0.f, -theight);
		}
		else
		{
			mat_translate(0.f, 0.f, -theight - 0.01f);
		}
	}
	else if (m_Selector.highlight_tile(this, vtile.tile))
	{
		mat_translate(0.f, 0.f, theight);
		m_Selector.display();
		mat_translate(0.f, 0.f, -theight);
	}
}

void Visibility::display_terrain(const VisibleTile& vtile) const
{
	mat_translate(vtile.trans.get(0), vtile.trans.get(1), vtile.trans.get(2));

	if (vtile.tile->terrain)
	{
		vtile.tile->terrain->display();
	}
}

void Visibility::display() const
{
	// Set up the transform
	mat_push();
	mat_custom_transform(m_Transform);

	// Display the base of the tiles
	mat_push();
	auto iter = m_VisibleTiles.begin();
	while (iter != m_VisibleTiles.end())
		display_tile(*iter++);
	mat_pop();

	// Display the object on each tile
	mat_push();
	iter = m_VisibleTiles.begin();
	while (iter != m_VisibleTiles.end())
		display_object(*iter++);
	mat_pop();

	// Display the terrain on each tile
	mat_push();
	iter = m_VisibleTiles.begin();
	while (iter != m_VisibleTiles.end())
		display_terrain(*iter++);
	mat_pop();

	// Clean up the transform
	mat_pop();
}





BattleState::BattleState(string map) : m_Grid(map), m_Visibility(m_Bounds, &m_Grid)
{
	m_Visibility.reset();

	unfreeze();
}

void BattleState::__set_bounds()
{
	m_Transform.set(0, 0, 2.f / get_width());
	m_Transform.set(1, 1, 2.f / get_height());
	m_Transform.set(2, 2, 0.0001f);
}

void BattleState::__display() const
{
	mat_push();
	mat_custom_transform(m_Transform);

	// Draw the grid
	m_Visibility.display();
	
	mat_pop();
}

void BattleState::__update(int frames_passed)
{
	m_Visibility.update(frames_passed);
	m_Queue.update();
}

void BattleState::freeze()
{
	UpdateListener::freeze();
	KeyboardListener::freeze();
}

void BattleState::unfreeze()
{
	UpdateListener::unfreeze();
	KeyboardListener::unfreeze();
}

int BattleState::trigger(const KeyEvent& event_data)
{
	if (event_data.pressed)
	{
		// Controls to rotate the camera
		if (event_data.control == CONTROL_ROTATE_LEFT)
		{
			m_Visibility.rotate_left();
			return EVENT_STOP;
		}
		if (event_data.control == CONTROL_ROTATE_RIGHT)
		{
			m_Visibility.rotate_right();
			return EVENT_STOP;
		}

		if (m_Phase)
		{
			if (false) // Ally has been selected
			{
				switch (event_data.control)
				{
				case CONTROL_CANCEL:
					//m_SelectedAlly = nullptr;
					break;
				}
			}
			else
			{
				switch (event_data.control)
				{
				// Controls to move the selected tile
				case CONTROL_MOVE_LEFT:
					m_Visibility.adjust_selected_tile(-1, 0);
					break;
				case CONTROL_MOVE_RIGHT:
					m_Visibility.adjust_selected_tile(1, 0);
					break;
				case CONTROL_MOVE_DOWN:
					m_Visibility.adjust_selected_tile(0, -1);
					break;
				case CONTROL_MOVE_UP:
					m_Visibility.adjust_selected_tile(0, 1);
					break;

				// Select a character to use
				case CONTROL_SELECT:
				{
					/*
					Tile* t = m_Selector.get_tile(this);
					if (t)
					{
						// Check if selected tile contains ally character, select that character if so
					}
					*/
					break;
				}
				}
			}
		}
	}

	return EVENT_CONTINUE;
}



bool Battle::Object::m_IsObjectDataLoaded{ false };

unordered_map<string, unordered_map<string, string>*> Battle::Object::m_ObjectData{};

unordered_map<string, Battle::Object*> Battle::Object::m_Objects{};

Battle::Object::Object(std::string id)
{
	m_Objects.emplace(id, this);
}

Battle::Object* Battle::Object::get_object(std::string id)
{
	// Check if the object has already been loaded.
	auto iter = m_Objects.find(id);
	if (iter != m_Objects.end())
		return iter->second;

	// Load the object data, if it hasn't been loaded already.
	if (!m_IsObjectDataLoaded)
	{
		LoadFile file("res/data/objects.txt");

		while (file.good())
		{
			unordered_map<string, string>* data = new unordered_map<string, string>();
			string id = file.load_data(*data);
			m_ObjectData.emplace(id, data);
		}

		m_IsObjectDataLoaded = true;
	}

	// Load the object
	Battle::Object* obj = nullptr;

	auto data_iter = m_ObjectData.find(id);
	if (data_iter != m_ObjectData.end())
	{
		unordered_map<string, string>& data = *data_iter->second;
		string type = data["type"];

		// StaticBattleObject
		if (type.compare("static") == 0)
		{
			obj = new StaticObject(id, data["sprite_sheet"], data["sprite"]);
		}

		if (obj)
			m_Objects.emplace(id, obj);

		// Clean up
		delete data_iter->second;
		m_ObjectData.erase(id);
	}

	return obj;
}


BillboardedObject::BillboardedObject(string id, SpriteGraphic* sprite) : Battle::Object(id)
{
	m_Sprite = sprite;
}

void BillboardedObject::display() const
{
	mat_push();
	mat_translate(0.5f * GRID_TILE_SIZE, 0.5f * GRID_TILE_SIZE, 0.0001f);
	//mat_scale(1.154700538f, 1.f, 1.f);
	//mat_rotatez(-0.5235987756f);
	mat_rotatez(-Visibility::get_angle());
	mat_translate(-0.5f * m_Sprite->get_width(), -0.15f * m_Sprite->get_width(), 0.f);
	mat_rotatex(1.5708f);
	m_Sprite->display();
	mat_pop();
}


StaticObject::StaticObject(string id, string sprite_sheet, string sprite) : BillboardedObject(id, nullptr)
{
	SpriteSheet* ssheet = SpriteSheet::generate(sprite_sheet.c_str());
	Sprite* spr = Sprite::get_sprite(sprite);

	Palette* palette = new SinglePalette(vec4f(1.f, 0.f, 0.f, 0.f), vec4f(0.f, 1.f, 0.f, 0.f), vec4f(0.f, 0.f, 1.f, 0.f));
	m_Sprite = new StaticSpriteGraphic(ssheet, spr, palette);
}




int Event::start()
{
	return EVENT_STOP;
}

int Event::update()
{
	return EVENT_STOP;
}


void Queue::push(Event* event, int priority)
{
	if (m_Queue.empty())
	{
		m_Queue.push_back({ event, priority });
	}
	else
	{
		// If priority is closer to front than back, start searching from the front
		if (priority - m_Queue.front().priority < m_Queue.back().priority - priority)
		{
			// If priority is before the front, insert before the first item
			if (priority < m_Queue.front().priority)
			{
				m_Queue.push_front({ event, priority });
			}

			// Search the list for where to insert it
			else
			{
				for (auto iter = m_Queue.begin(); iter != m_Queue.end(); ++iter)
				{
					if (priority < iter->priority)
					{
						m_Queue.insert(iter, { event, priority });
						break;
					}
				}
			}
		}

		// Start searching from the back
		else
		{
			// If priority is after the back, insert after the last item
			if (priority > m_Queue.back().priority)
			{
				m_Queue.push_back({ event, priority });
			}

			// Search the list for where to insert it
			else
			{
				auto iter = m_Queue.end();
				while (iter != m_Queue.begin())
				{
					--iter;
					if (priority >= iter->priority)
					{
						m_Queue.insert(++iter, { event, priority });
						break;
					}
				}
			}
		}
	}
}

Event* Queue::pop()
{
	if (m_Queue.empty())
		return nullptr;

	Event* e = m_Queue.front().event;
	m_Queue.pop_front();
	return e;
}

void Queue::update()
{
	if (m_Current)
	{
		// If the current event has reached its end, pop the next event from the queue
		if (m_Current->update() == EVENT_STOP)
		{
			// Loop while the next event ends after its start function
			while ((m_Current = pop()) != nullptr && m_Current->start() == EVENT_STOP) {}
		}
	}
}