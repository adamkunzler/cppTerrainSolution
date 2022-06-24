#ifndef CHUNK_H
#define CHUNK_H

struct NoiseSettings
{
	unsigned int octaves{ 6 };
	float redistribution{ 3.f };
	float noiseScale{ 0.015f };
	float startFrequency{ 1.f };
	float startAmplitude{ 1.f };
};

class NoiseGenerator
{
private:
	std::unique_ptr<OpenSimplexNoise> _simplex;

	float _octaveOffset = 0.5f;
	unsigned int _octaves{ 6 };
	float _redistribution{ 3.f };
	float _noiseScale{ 0.015f };

public:
	NoiseGenerator(long seed)
	{
		_simplex = std::make_unique<OpenSimplexNoise>(seed);
	}

	void generateNoise(
		std::vector<float>& heightmap,
		unsigned int width, unsigned int height,
		float startX, float startY
	)
	{
		heightmap.clear();

		for (unsigned int y = 0; y < height; ++y)
		{
			for (unsigned int x = 0; x < width; ++x)
			{
				float xx{ (startX + x) * _noiseScale };
				float yy{ (startY + y) * _noiseScale };

				float height = getNoiseAt(xx, yy);
				heightmap.emplace_back(height);
			}
		}
	}

	float getNoiseAt(float x, float y)
	{
		float noise{ 0.f };

		float frequency{ 1.f };
		float amplitude{ 1.f };
		float sumAmplitude{ 0.f };

		for (unsigned int octave = 0; octave < _octaves; ++octave)
		{
			noise += _simplex->eval(
				frequency * (x + (_octaveOffset * octave)),
				frequency * (y + (_octaveOffset * octave))
			) * amplitude;

			sumAmplitude += amplitude;
			frequency *= 2.f;
			amplitude *= 0.5f;
		}

		noise /= sumAmplitude;
		noise = (noise + 1.f) / 2.f; // map between 0 and 1
		noise = std::powf(noise * 1.2f, _redistribution);

		if (noise > 1.f) noise = 1.f;
		if (noise < 0.f) noise = 0.f;

		return noise;
	}
};

class Chunk
{
private:
	NoiseGenerator _heightmapGenerator{ 0 };

	int _x{ 0 };
	int _y{ 0 };
	int _width{ 0 };
	int _height{ 0 };
	int _scale{ 0 };
	bool _drawBorder{ false };

	std::vector<float> _heightData;
	sf::VertexArray _vertices;

	sf::RectangleShape _border;

	std::unique_ptr<sf::RenderTexture> _texture;
	sf::Sprite _sprite;

	sf::Color getColor(unsigned int x, unsigned int y)
	{
		sf::Uint8 val = static_cast<sf::Uint8>(_heightData[x + y * _width] * 255.f);
		return sf::Color{ val, val, val, 255 };
	}

public:

	int getX() const { return _x; }
	int getY() const { return _y; }
	bool& drawBorder() { return _drawBorder; }

	Chunk(int x, int y, int width, int height, int scale)
		: _x{ x }, _y{ y }, _width{ width + 1 }, _height{ height + 1 }, _scale{ scale }
	{
		_heightmapGenerator.generateNoise(_heightData,
			_width, _height,
			_x* (_width - 1), // offset x and y by width/height for chunk tiling (minus 1 to remove artifact)
			_y* (_height - 1));

		std::size_t numVertices = static_cast<std::size_t>(_width * _height * 4);
		_vertices = sf::VertexArray{ sf::Quads, numVertices };

		_border.setSize(sf::Vector2f{ static_cast<float>(width * _scale), static_cast<float>(height * _scale) });
		_border.setOutlineThickness(-2.5f);
		_border.setOutlineColor(sf::Color{ 235, 121, 46, 255 });
		_border.setFillColor(sf::Color{ 0,0,0,0 });

		_texture = std::make_unique<sf::RenderTexture>();
		if (!_texture->create(width * scale, height * scale))
		{
			std::cout << "Couldn't create chunk texture!" << std::endl;
			return;
		}

		setupTextureFromVertices();
	}

	bool inBounds(float x, float y)
	{
		if (x >= _x && x < _x + 1.f && y >= _y && y < _y + 1.f)
			return true;

		return false;
	}

	// todo make private
	void setupTextureFromVertices()
	{
		_vertices.clear();

		for (unsigned int y = 0; y < _height - 1; ++y)
		{
			for (unsigned int x = 0; x < _width - 1; ++x)
			{
				float x1 = static_cast<float>(x * _scale);// +position.x;
				float y1 = static_cast<float>(y * _scale);// +position.y;

				float x2 = static_cast<float>((x + 1) * _scale);// +position.x;
				float y2 = static_cast<float>((y + 1) * _scale);// +position.y;

				// check bounds
				/*if ((x2 < 0) || (x1 > window.getSize().x) || (y2 < 0) || (y1 > window.getSize().y))
				{
					continue;
				}*/

				sf::Vertex v1
				{
					sf::Vector2f{x1, y1},
					getColor(x, y)
				};

				sf::Vertex v2
				{
					sf::Vector2f{x2 , y1},
					getColor(x + 1, y)
				};

				sf::Vertex v3
				{
					sf::Vector2f{x2, y2},
					getColor(x + 1, y + 1)
				};

				sf::Vertex v4
				{
					sf::Vector2f{x1, y2},
					getColor(x, y + 1)
				};

				_vertices.append(v1); _vertices.append(v2); _vertices.append(v3); _vertices.append(v4);
			}
		}

		_texture->draw(_vertices);
		_texture->display();
		_sprite.setTexture(_texture->getTexture());
		//_sprite.setScale(_scale, _scale);
	}

	void render(sf::RenderWindow& window, const sf::Vector2f& position)
	{
		_sprite.setPosition(position);
		window.draw(_sprite);
		//window.draw(_vertices);

		if (_drawBorder)
		{
			_border.setPosition(position);
			window.draw(_border);
		}
	}
};

#endif