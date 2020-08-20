#include <algorithm>
#include <regex>
#include "../../include/controls.h"
#include "../../include/battle.h"

#include <iostream>

#define GRID_COORDINATE(x, y, width) ((x) + ((width) * (y)))

using namespace std;


std::unordered_map<string, BattleTileSet*> BattleTileSet::m_Sets{};

BattleTileSet::BattleTileSet(string id)
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
				BattleTileType* type = new BattleTileType();
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
				BattleTileType* type = new BattleTileType();
				type->side = Sprite::get_sprite(line);
				m_Types.emplace(match[1].str(), type);
			}
		}
	}
}

BattleTileSet* BattleTileSet::get_tile_set(string id)
{
	auto iter = m_Sets.find(id);
	if (iter != m_Sets.end())
		return iter->second;
	
	BattleTileSet* s = new BattleTileSet(id);
	m_Sets.emplace(id, s);
	return s;
}

SpriteSheet* BattleTileSet::get_sprite_sheet()
{
	return m_SpriteSheet;
}

const BattleTileType* BattleTileSet::get_tile_type(string type) const
{
	auto iter = m_Types.find(type);
	if (iter != m_Types.end())
		return iter->second;
	return nullptr;
}



BattleGrid::BattleGrid(string map)
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
			m_TileSet = BattleTileSet::get_tile_set(match[1].str());
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

				BattleTile* new_tiles = new BattleTile[new_width * new_height];
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
			const BattleTileType* type = m_TileSet->get_tile_type(match[1].str());

			for (int i = x; i < x + dx; ++i)
			{
				for (int j = y; j < y + dy; ++j)
				{
					BattleTile* tile = get_tile(i, j);
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

			BattleTile* tile = get_tile(x, y);
			if (tile)
				tile->obj = BattleObject::get_object(match[1].str());

			cout << "Object loaded.";
		}
	}
}

BattleTile* BattleGrid::get_tile(int x, int y)
{
	if (x >= 0 && x < width && y >= 0 && y < height)
		return m_Tiles + GRID_COORDINATE(x, y, width);
	return nullptr;
}

const BattleTile* BattleGrid::get_tile(int x, int y) const
{
	if (x >= 0 && x < width && y >= 0 && y < height)
		return m_Tiles + GRID_COORDINATE(x, y, width);
	return nullptr;
}

void BattleGrid::display(int dx, int dy, Palette* palette) const
{
	SpriteSheet* tile_sheet = m_TileSet->get_sprite_sheet();

	int istart, iend, jstart, jend;
	if (dx < 0)
	{
		istart = width - 1;
		iend = -1;
	}
	else
	{
		istart = 0;
		iend = width;
	}
	if (dy < 0)
	{
		jstart = height - 1;
		jend = -1;
	}
	else
	{
		jstart = 0;
		jend = height;
	}

	mat_translate(GRID_TILE_SIZE * istart, GRID_TILE_SIZE * jstart, 0.f);
	for (int i = istart; i != iend; i += dx)
	{
		mat_push();
		for (int j = jstart; j != jend; j += dy)
		{
			// Check if tile exists
			if (const BattleTile* tile = get_tile(i, j))
			{
				// TODO check if on-screen

				// Draw the ground of the tile
				mat_push();
				mat_translate(0.f, 0.f, tile->height * GRID_TILE_HEIGHT);
				tile_sheet->display(tile->type->top->key, palette);

				// Draw the sides of the tile
				const BattleTile* tx = get_tile(i + dx, j);
				const BattleTile* ty = get_tile(i, j + dy);

				int dh = tile->height - (tx ? tx->height : 0);

				if (dh > 0)
				{
					mat_push();

					if (dx > 0)
					{
						mat_translate(GRID_TILE_SIZE, GRID_TILE_SIZE, 0.f);
						mat_scale(1.f, -1.f, 1.f);
					}

					mat_rotatez(1.5708f);
					mat_rotatex(1.5708f);

					for (int k = 0; k < dh; ++k)
					{
						mat_translate(0.f, -GRID_TILE_HEIGHT, 0.f);
						tile_sheet->display(tile->type->side->key, palette);
					}

					mat_pop();
				}

				dh = tile->height - (ty ? ty->height : 0);

				if (dh > 0)
				{
					mat_push();

					if (dy > 0)
					{
						mat_translate(GRID_TILE_SIZE, GRID_TILE_SIZE, 0.f);
						mat_scale(-1.f, 1.f, 1.f);
					}

					mat_rotatex(1.5708f);

					for (int k = 0; k < dh; ++k)
					{
						mat_translate(0.f, -GRID_TILE_HEIGHT, 0.f);
						tile_sheet->display(tile->type->side->key, palette);
					}

					mat_pop();
				}

				// Draw the terrain of the tile
				if (tile->terrain)
					tile->terrain->display();

				// Draw the object on the tile
				if (tile->obj)
				{
					int terrain_height = tile->terrain ? tile->terrain->get_height() : 0;
					mat_translate(0.f, 0.f, terrain_height * GRID_TILE_HEIGHT);
					tile->obj->display();
				}
				mat_pop();
			}

			mat_translate(0.f, GRID_TILE_SIZE * dy, 0.f);
		}
		mat_pop();
		mat_translate(GRID_TILE_SIZE * dx, 0.f, 0.f);
	}
}




#define ROTATE_SPEED	0.01309f
#define HALF_PI			1.570796327f

float BattleState::m_Angle{ 0.7854f };

BattleState::BattleState(string map) : m_Grid(map)
{
	m_TargetAngle = m_Angle - HALF_PI;

	m_Camera.set(0, 0, 256.f);
	m_Camera.set(1, 0, 256.f);

	m_Zoom = 1.f;

	unfreeze();
}

void BattleState::__set_bounds()
{
	m_Transform.set(0, 0, 2.f * m_Zoom / get_width());
	m_Transform.set(1, 1, 2.f * m_Zoom / get_height());
	m_Transform.set(2, 2, 0.0001f);
}

void BattleState::__display() const
{
	Palette* clear_palette = new SinglePalette(vec4f(1.f, 0.f, 0.f, 0.f), vec4f(0.f, 1.f, 0.f, 0.f), vec4f(0.f, 0.f, 1.f, 0.f));

	mat_push();
	mat_custom_transform(m_Transform);

	// Draw the grid
	mat_rotatex(0.8727f);
	mat_scale(1.f, 1.f, -1.3054f);
	mat_rotatez(m_Angle);
	mat_translate(-m_Camera.get(0), -m_Camera.get(1), 0.f);
	m_Grid.display(sinf(m_Angle) > 0 ? -1 : 1, cosf(m_Angle) > 0 ? -1 : 1, clear_palette);

	mat_pop();
}

void BattleState::__update(int frames_passed)
{
	if (m_TargetAngle < m_Angle)
	{
		m_Angle -= frames_passed * ROTATE_SPEED;
		if (m_Angle < m_TargetAngle) 
			m_Angle = m_TargetAngle;
	}
	else if (m_TargetAngle > m_Angle)
	{
		m_Angle += frames_passed * ROTATE_SPEED;
		if (m_Angle > m_TargetAngle)
			m_Angle = m_TargetAngle;
	}
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
		if (event_data.control == CONTROL_ROTATE_LEFT)
		{
			if (m_Angle <= m_TargetAngle)
				m_TargetAngle -= HALF_PI;
			return EVENT_STOP;
		}
		else if (event_data.control == CONTROL_ROTATE_RIGHT)
		{
			if (m_Angle >= m_TargetAngle)
				m_TargetAngle += HALF_PI;
			return EVENT_STOP;
		}
	}

	return EVENT_CONTINUE;
}

float BattleState::get_angle()
{
	return m_Angle;
}



bool BattleObject::m_IsObjectDataLoaded{ false };

unordered_map<string, unordered_map<string, string>*> BattleObject::m_ObjectData{};

unordered_map<string, BattleObject*> BattleObject::m_Objects{};

BattleObject::BattleObject(std::string id)
{
	m_Objects.emplace(id, this);
}

BattleObject* BattleObject::get_object(std::string id)
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
	BattleObject* obj = nullptr;

	auto data_iter = m_ObjectData.find(id);
	if (data_iter != m_ObjectData.end())
	{
		unordered_map<string, string>& data = *data_iter->second;
		string type = data["type"];

		// StaticBattleObject
		if (type.compare("static") == 0)
		{
			obj = new StaticBattleObject(id, data["sprite_sheet"], data["sprite"]);
		}

		if (obj)
			m_Objects.emplace(id, obj);

		// Clean up
		delete data_iter->second;
		m_ObjectData.erase(id);
	}

	return obj;
}


BillboardedBattleObject::BillboardedBattleObject(string id, SpriteGraphic* sprite) : BattleObject(id)
{
	m_Sprite = sprite;
}

void BillboardedBattleObject::display() const
{
	mat_push();
	mat_translate(0.5f * GRID_TILE_SIZE, 0.5f * GRID_TILE_SIZE, 0.f);
	mat_rotatez(-BattleState::get_angle());
	mat_translate(-0.5f * m_Sprite->get_width(), 0.f, 0.f);
	mat_rotatex(1.5708f);
	m_Sprite->display();
	mat_pop();
}


StaticBattleObject::StaticBattleObject(string id, string sprite_sheet, string sprite) : BillboardedBattleObject(id, nullptr)
{
	SpriteSheet* ssheet = SpriteSheet::generate(sprite_sheet.c_str());
	Sprite* spr = Sprite::get_sprite(sprite);

	Palette* palette = new SinglePalette(vec4f(1.f, 0.f, 0.f, 0.f), vec4f(0.f, 1.f, 0.f, 0.f), vec4f(0.f, 0.f, 1.f, 0.f));
	m_Sprite = new StaticSpriteGraphic(ssheet, spr, palette);
}