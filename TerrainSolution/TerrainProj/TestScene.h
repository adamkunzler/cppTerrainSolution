#pragma once

#include <thread>
#include <mutex>

#include "terrain/chunk.h"

class BaseScene;

class TestScene : public BaseScene
{
private:
	sf::Vector2f _camera{ 0.f, 0.f };
	float _cameraRadius{ 25.f };
	sf::CircleShape _cameraShape;
	float _cameraSpeed{ 1000.f };

	bool _isArrowKeyDown{ false };
	bool _isArrowKeyUp{ false };
	bool _isArrowKeyLeft{ false };
	bool _isArrowKeyRight{ false };

	int _chunkWidth{ 64 };
	int _chunkHeight{ 64 };
	int _chunkScale{ 4 };

	std::vector<Chunk> _chunks;

	int _minChunkColumn{ -1 };
	int _maxChunkColumn{ 1 };
	int _minChunkRow{ -1 };
	int _maxChunkRow{ 1 };

	std::mutex _mutex;
	std::thread _createChunksThread;

	void createChunks(int minX, int maxX, int minY, int maxY)
	{		
		for (int y = minY; y <= maxY; ++y)
		{
			for (int x = minX; x <= maxX; ++x)
			{
				_chunks.emplace_back(Chunk{ x, y, _chunkWidth, _chunkHeight, _chunkScale });
			}
		}			
	}
	
public:
	TestScene(int width, int height, std::string title)
		: BaseScene{ width, height, title }
	{
		// init chunks
		createChunks(_minChunkColumn, _maxChunkColumn, _minChunkRow, _maxChunkRow);
		
		// init camera
		_camera.x = 0;
		_camera.y = 0;

		_cameraShape.setPosition(halfWidth - _cameraRadius, halfHeight - _cameraRadius); // middle of the screen...always
		_cameraShape.setRadius(25.f);
		_cameraShape.setFillColor(sf::Color{ 64, 157, 74 , 255 });
	}

	void processSceneEvents(const sf::Event& event)
	{
		if (event.type == sf::Event::KeyPressed)
		{
			if (event.key.code == sf::Keyboard::W)
				_isArrowKeyUp = true;
			if (event.key.code == sf::Keyboard::S)
				_isArrowKeyDown = true;
			if (event.key.code == sf::Keyboard::A)
				_isArrowKeyLeft = true;
			if (event.key.code == sf::Keyboard::D)
				_isArrowKeyRight = true;
		}

		if (event.type == sf::Event::KeyReleased)
		{
			if (event.key.code == sf::Keyboard::W)
				_isArrowKeyUp = false;
			if (event.key.code == sf::Keyboard::S)
				_isArrowKeyDown = false;
			if (event.key.code == sf::Keyboard::A)
				_isArrowKeyLeft = false;
			if (event.key.code == sf::Keyboard::D)
				_isArrowKeyRight = false;
		}

		if (event.type == sf::Event::MouseButtonPressed)
		{
			auto mx = sf::Mouse::getPosition(window).x;
			auto my = sf::Mouse::getPosition(window).y;

			auto cameraWorldCoords = getWorldCoords(mx, my);
			auto cameraScreenCoords = getScreenCoords(mx, my);
			auto cameraChunkCoords = getChunkCoords(mx, my);

			std::cout << "Mouse (world): " << cameraWorldCoords.x << ", " << cameraWorldCoords.y << std::endl;
			std::cout << "Mouse (screen): " << cameraScreenCoords.x << ", " << cameraScreenCoords.y << std::endl;
			std::cout << "Mouse (chunk): " << cameraChunkCoords.x << ", " << cameraChunkCoords.y << std::endl;
			std::cout << std::endl;
		}
	}

	// gets the translated screen coords (e.g. 0,0 is center of screen)
	sf::Vector2f getScreenCoords(float screenX, float screenY)
	{
		return sf::Vector2f{
			screenX - halfWidth,
			screenY - halfHeight
		};
	}

	sf::Vector2f getWorldCoords(float screenX, float screenY)
	{
		return sf::Vector2f{
			_camera.x + (screenX - halfWidth),
			_camera.y + (screenY - halfHeight)
		};
	}

	sf::Vector2f getChunkCoords(float screenX, float screenY)
	{
		sf::Vector2f world = getWorldCoords(screenX, screenY);

		float chunkWidth = _chunkWidth * _chunkScale;
		float chunkHeight = _chunkHeight * _chunkScale;

		float chunkX = (world.x + (chunkWidth / 2.f)) / chunkWidth;
		float chunkY = (world.y + (chunkHeight / 2.f)) / chunkHeight;

		return sf::Vector2f{ chunkX, chunkY };
	}

	sf::Vector2f getCameraChunkCoords()
	{
		return getChunkCoords(
			_cameraShape.getPosition().x + _cameraRadius,
			_cameraShape.getPosition().y + _cameraRadius
		);
	}

	sf::Vector2i getCurrentChunkCoords()
	{
		auto coords = getChunkCoords(
			_cameraShape.getPosition().x + _cameraRadius,
			_cameraShape.getPosition().y + _cameraRadius
		);

		return sf::Vector2i{
			(int)std::floor(coords.x),
			(int)std::floor(coords.y)
		};
	}

	void cleanupChunks()
	{
		auto removeStart = std::remove_if(_chunks.begin(), _chunks.end(),
			[this](const Chunk& chunk) {
				return chunk.getX() < _minChunkColumn || chunk.getX() > _maxChunkColumn
					|| chunk.getY() < _minChunkRow || chunk.getY() > _maxChunkRow;
			});
		_chunks.erase(removeStart, _chunks.end());
	}

	void updateScene(sf::Time elapsed) override
	{
		if (_isArrowKeyUp || _isArrowKeyDown || _isArrowKeyLeft || _isArrowKeyRight)
		{
			if (_isArrowKeyUp) _camera.y -= _cameraSpeed * elapsed.asSeconds();
			if (_isArrowKeyDown) _camera.y += _cameraSpeed * elapsed.asSeconds();
			if (_isArrowKeyLeft) _camera.x -= _cameraSpeed * elapsed.asSeconds();
			if (_isArrowKeyRight) _camera.x += _cameraSpeed * elapsed.asSeconds();
		}

		updateChunkBorder();

		auto chunksNeedCleanup = checkChunkBoundaries();
		if (chunksNeedCleanup)
		{
			cleanupChunks();
		}
	}

	bool checkChunkBoundaries()
	{
		auto currentChunk = getCurrentChunkCoords();

		// check each direction and check if a neighbor chunk exists...if not
		// create a new column/row of chunks and delete the far away chunks

		// check left
		if (currentChunk.x == _minChunkColumn)
		{
			// create left
			_minChunkColumn--;
			createChunks(_minChunkColumn, _minChunkColumn, _minChunkRow, _maxChunkRow);

			// destroy right
			_maxChunkColumn--;
			return true;
		}

		// check right
		if (currentChunk.x == _maxChunkColumn)
		{
			// create right
			_maxChunkColumn++;
			createChunks(_maxChunkColumn, _maxChunkColumn, _minChunkRow, _maxChunkRow);

			// destroy left
			_minChunkColumn++;
			return true;
		}

		// check top
		if (currentChunk.y == _minChunkRow)
		{
			// create top
			_minChunkRow--;
			createChunks(_minChunkColumn, _maxChunkColumn, _minChunkRow, _minChunkRow);

			// destroy bottom
			_maxChunkRow--;
			return true;
		}

		// check bottom
		if (currentChunk.y == _maxChunkRow)
		{
			// create bottom
			_maxChunkRow++;
			createChunks(_minChunkColumn, _maxChunkColumn, _maxChunkRow, _maxChunkRow);

			// destroy top
			_minChunkRow++;
			return true;
		}

		return false;
	}

	void updateChunkBorder()
	{
		// update to show border around current chunk
		auto chunkCoords = getCameraChunkCoords();
		for (auto& chunk : _chunks)
		{
			// maybe update this to use integer coords to compare against _x and _y?
			chunk.drawBorder() = chunk.inBounds(chunkCoords.x, chunkCoords.y);
		}
	}

	void drawScene(sf::Time elapsed) override
	{
		float chunkWidth = _chunkWidth * _chunkScale;
		float chunkHeight = _chunkHeight * _chunkScale;

		float worldTranslateX = halfWidth - (chunkWidth / 2.f);
		float worldTranslateY = halfHeight - (chunkHeight / 2.f);

		for (auto& chunk : _chunks)
		{
			float x = (chunk.getX() * chunkWidth) + worldTranslateX - _camera.x;
			float y = (chunk.getY() * chunkHeight) + worldTranslateY - _camera.y;
			chunk.render(window, sf::Vector2f{ x, y });
		}

		window.draw(_cameraShape);
	}

	std::vector<std::string> getOverlayMessages()
	{
		std::vector<std::string> msg;

		auto chunk = getCameraChunkCoords();

		msg.push_back("Camera (world): " + std::to_string(_camera.x) + ", " + std::to_string(_camera.y));
		msg.push_back("Camera (screen): " + std::to_string(0) + ", " + std::to_string(0));
		msg.push_back("Camera (chunk): " + std::to_string(chunk.x) + ", " + std::to_string(chunk.y));

		auto chunkI = getCurrentChunkCoords();
		msg.push_back("Chunk: " + std::to_string(chunkI.x) + ", " + std::to_string(chunkI.y));

		return msg;
	}

	void customImGui()
	{
	}
};