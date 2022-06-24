#pragma once

class BaseScene
{
protected:
	int width{0};
	int height{0};	
	float halfWidth{ 0.0f };
	float halfHeight{ 0.0f };
	
	sf::RenderWindow window;

	sf::Text txtFps;
	sf::Font font;
		
	virtual void drawScene(sf::Time elapsed) = 0;
	virtual void updateScene(sf::Time elapsed) = 0;
	virtual void processSceneEvents(const sf::Event& event) = 0;	

public:
	virtual std::vector<std::string> getOverlayMessages() = 0;
	virtual void customImGui() {};

	BaseScene(int width, int height, std::string title)
		: width{ width }, height{ height }, halfWidth{ (float)width / 2.0f }, halfHeight{ (float)height / 2.0f }
	{
		window.create(sf::VideoMode(width, height), title);	

		if (font.loadFromFile("assets/CourierPrime-Regular.ttf"))
		{
			txtFps.setFont(font);
			txtFps.setCharacterSize(18);
			txtFps.setFillColor(sf::Color::White);
			txtFps.setPosition(10, 10);
		}		
	}

	~BaseScene() {}

	void processEvents(const sf::Event &event)
	{
		processSceneEvents(event);
	}

	void update(sf::Time elapsed)
	{		
		updateScene(elapsed);
	}
	
	void draw(sf::Time elapsed)
	{		
		drawScene(elapsed);
	}
	
	sf::RenderWindow& getWindow()
	{
		return window;
	}

	
};