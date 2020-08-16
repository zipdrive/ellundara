#include <algorithm>
#include "../../include/battle.h"

#include <iostream>

#define GRID_COORDINATE(x, y, width) ((x) + ((width) * (y)))

using namespace std;


SpriteSheet* BattleTile::sprite_sheet{ nullptr };


BattleGrid::BattleGrid(string map)
{
	if (!BattleTile::sprite_sheet)
	{
		BattleTile::sprite_sheet = SpriteSheet::generate_empty();
		BattleTile::sprite_sheet->load_partitioned_sprite_sheet("tiles.png", GRID_TILE_SIZE, GRID_TILE_SIZE);
	}

	width = 0;
	height = 0;
	m_Tiles = nullptr;

	LoadFile file("res/maps/" + map + ".txt");

	while (file.good())
	{
		std::unordered_map<string, int> line;
		string id = file.load_data(line);

		if (line.find("x") != line.end())
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

			SPRITE_KEY top = line["top"];
			SPRITE_KEY side = line["side"];

			for (int i = x; i < x + dx; ++i)
			{
				for (int j = y; j < y + dy; ++j)
				{
					BattleTile* tile = get_tile(i, j);
					tile->top = top;
					tile->side = side;
					tile->height = h;
				}
			}
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



BattleState::BattleState(string map) : m_Grid(map)
{
	m_Camera.set(1, 0, 256.f);
	//m_Angle = 0.7854f;
	m_Angle = 2.3562f;
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
	mat_rotatex(0.8727f);
	mat_rotatez(m_Angle);

	vec2i dir(sinf(m_Angle) > 0 ? -1 : 1, cosf(m_Angle) > 0 ? -1 : 1);
	int istart, iend, jstart, jend;
	if (dir.get(0) < 0)
	{
		istart = m_Grid.width - 1;
		iend = -1;
	}
	else
	{
		istart = 0;
		iend = m_Grid.width;
	}
	if (dir.get(1) < 0)
	{
		jstart = m_Grid.height - 1;
		jend = -1;
	}
	else
	{
		jstart = 0;
		jend = m_Grid.height;
	}

	mat_translate((GRID_TILE_SIZE * istart) - m_Camera.get(0), (GRID_TILE_SIZE * jstart) - m_Camera.get(1), 0.f);
	for (int i = istart; i != iend; i += dir.get(0))
	{
		mat_push();
		for (int j = jstart; j != jend; j += dir.get(1))
		{
			// TODO check if tile needs to be drawn

			// Draw the ground of the tile
			mat_push();
			const BattleTile* tile = m_Grid.get_tile(i, j);
			mat_translate(0.f, 0.f, -tile->height * 0.3f * GRID_TILE_SIZE);
			BattleTile::sprite_sheet->display(tile->top, clear_palette);
			mat_pop();

			int dh;

			if (const BattleTile* dx = m_Grid.get_tile(i + dir.get(0), j))
			{
				if ((dh = tile->height - dx->height) > 0)
				{

				}
			}

			//const BattleTile* dy = m_Grid.get_tile(i, j + dir.get(1));
			
			mat_translate(0.f, GRID_TILE_SIZE * dir.get(1), 0.f);
		}
		mat_pop();
		mat_translate(GRID_TILE_SIZE * dir.get(0), 0.f, 0.f);
	}

	mat_pop();
}

void BattleState::__update(int frames_passed)
{
	m_Angle += frames_passed * 0.01309f;
}