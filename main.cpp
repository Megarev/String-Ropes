#include <SFML/Graphics.hpp>
#include "GraphicsRender.h"

class Player {
private:
	sf::Vector2f position, velocity;
	sf::RectangleShape box;
	float size;
	float gSpeed, gMax, jumpSpeed, moveSpeed;

	bool isContact;

	bool TileMapCollision(const Level& level) {
		int tileLeft = (int)(position.x / size);
		int tileRight = (int)ceilf((position.x + size) / size);
		int tileTop = (int)(position.y / size);
		int tileBottom = (int)ceilf((position.y + size) / size);
	
		for (int i = tileTop; i < tileBottom; i++) {
			for (int j = tileLeft; j < tileRight; j++) {
				switch (level.GetCharacter(j, i)) {
				case '#':
					return true;
					break;
				}
			}
		}

		return false;
	}
public:
	Player() {
		size = 32.0f;

		velocity = { 4.0f, 4.0f };
		box.setSize({ size, size });
		box.setFillColor(sf::Color::Blue);
	
		isContact = false;
		gSpeed = 2.0f;
		gMax = 4.0f;
		jumpSpeed = 25.0f;
		moveSpeed = 4.0f;
	}

	void Logic(const Level& level) {
		sf::Vector2f initPosition;

		initPosition.x = position.x;
		position.x += velocity.x;
		bool isCollideX = TileMapCollision(level);
		if (isCollideX) {
			position.x = initPosition.x;
		}

		initPosition.y = position.y;

		if (!isContact) {
			velocity.y += gSpeed;
			velocity.y = std::fminf(gMax, velocity.y);
		}
		isContact = false;

		position.y += velocity.y;
		bool isCollideY = TileMapCollision(level);
		if (isCollideY) {
			position.y = initPosition.y;
			position.y -= ((int)initPosition.y % (int)size);
			if (velocity.y > 0.0f) isContact = true;
		}
	
		box.setPosition(position);
	}

	void Render(sf::RenderWindow& window) {
		window.draw(box);
	}

	inline sf::Vector2f GetPosition() const { return position; }
	void SetPosition(const sf::Vector2f& pos) { position = pos; }

	void SetVelocity(int component, float value) {
		switch (component) {
		case 0:
			velocity.x = value;
			break;
		case 1:
			velocity.y = value;
			break;
		}
	}

	void Jump() {
		velocity.y = -jumpSpeed;
	}

	void HorizontalMove(int dir) {
		velocity.x = dir * moveSpeed;
	}

	bool& GetIsContact() { return isContact; }
};

class StringRopeMain {
public:
	sf::Vector2f position, points[4];
	float stringLength, elasticMax, stringStretch;
	bool isPlayerOnString;

	sf::Color color;

	enum StringType {
		StringRope = 0,
		StringBounce = 1
	};

	StringRopeMain() {
		stringLength = 192.0f;
		elasticMax = 32.0f;
		stringStretch = 0.0f;
		isPlayerOnString = false;
	}

	virtual void Logic(Player&) = 0;
	
	bool IsPositionInBounds(const sf::Vector2f& position) {
		auto [x, y] = position;
		return (y + 35.0f > points[1].y && y + 29.0f < points[1].y && x + 32 > position.x && x < position.x + stringLength);
	}

	void Render(sf::RenderWindow& window) {
		DrawLine(window, points[0].x, points[0].y, points[1].x, points[1].y, color);
		DrawLine(window, points[1].x, points[1].y, points[2].x, points[2].y, color);
		DrawLine(window, points[2].x, points[2].y, points[3].x, points[3].y, color);
	}

	inline sf::Vector2f GetPosition() const { return position; }
	void SetStringLength(float length) { stringLength = length; }
	void SetPosition(const sf::Vector2f& pos) { 
		position = pos; 
			
		points[0] = position;
		points[3] = { position.x + stringLength, position.y };
		points[1] = { points[3].x / 2.0f, position.y };
		points[2] = { points[1].x + 32.0f, position.y };
	}
};

class StringRope : public StringRopeMain {
public:
	StringRope() {
		color = sf::Color::White;
	}

	void Logic(Player& player) override {
		auto [x, y] = player.GetPosition();

		if (y + 35.0f > points[1].y && y + 29.0f < points[1].y && x + 32 > position.x && x < position.x + stringLength) {

			isPlayerOnString = true;

			float distance = x - position.x;
			points[1].x = points[0].x + distance;
			points[2].x = points[1].x + 32.0f;

			stringStretch += 2.0f;
			stringStretch = std::fminf(stringStretch, elasticMax);
			player.SetVelocity(1, -1.0f * std::fminf(2.0f, stringStretch));

			points[1].y = points[0].y + stringStretch;
			points[2].y = points[0].y + stringStretch;
		}
		else {
			isPlayerOnString = false;
			if (stringStretch > 0.0f) {
				stringStretch--;
				points[1].y = points[0].y + stringStretch;
				points[2].y = points[0].y + stringStretch;
			}
		}
	}
};

class StringBounce : public StringRopeMain {
private:
	bool isElasticMaxPoint;
	float jumpSpeed;
public:
	StringBounce() {
		isElasticMaxPoint = false;
		color = sf::Color::Magenta;
		elasticMax = 60.0f;
	
		jumpSpeed = 40.0f;
	}

	void Logic(Player& player) override {
		auto [x, y] = player.GetPosition();
	
		if (y + 35.0f > points[1].y && y + 29.0f < points[1].y && x + 32 > position.x && x < position.x + stringLength) {
		
			isPlayerOnString = true;

			float distance = x - points[0].x;

			points[1].x = points[0].x + distance;
			points[2].x = points[1].x + 32.0f;

			if (!isElasticMaxPoint) {
				stringStretch += 2.0f;
				stringStretch = std::fminf(stringStretch, elasticMax);
				player.SetVelocity(1, -1.0f * std::fminf(2.0f, stringStretch));
			}
			else {
				player.SetVelocity(1, -jumpSpeed);
				isElasticMaxPoint = false;
			}

			if (stringStretch >= elasticMax) {
				isElasticMaxPoint = true;
			}
		
			points[1].y = points[0].y + stringStretch;
			points[2].y = points[0].y + stringStretch;
		}
		else {
			isPlayerOnString = false;
			isElasticMaxPoint = false;
			if (stringStretch > 0.0f) {
				stringStretch -= 4.0f;
				points[1].y = points[0].y + stringStretch;
				points[2].y = points[0].y + stringStretch;
			}
		}
	}
};

typedef std::vector<std::unique_ptr<StringRopeMain>> StringRopesVector;
class LineEditor {
private:
	sf::Vector2i initMousePos, currentMousePos, newMousePos;
	int size;

	bool isPressed;
public:
	LineEditor() {
		size = 32;
		isPressed = false;
	}

	void ManageEvent(StringRopesVector& strings, int index, sf::Event e) {
		switch (e.type) {
		case sf::Event::MouseButtonPressed:
			switch (e.key.code) {
			case sf::Mouse::Left:
				initMousePos = { e.mouseButton.x, e.mouseButton.y };
				isPressed = true;
				break;
			}
			break;
		case sf::Event::MouseButtonReleased:
			switch (e.key.code) {
			case sf::Mouse::Left:
				newMousePos = { (int)(e.mouseButton.x / size) * size, (int)(initMousePos.y / size) * size };
				
				isPressed = false;

				sf::Vector2i initPos = initMousePos;
				initMousePos = { (int)(initPos.x / size) * size, (int)(initPos.y / size * size) };

				switch (index) {
				case StringRopeMain::StringRope:
					strings.push_back(std::make_unique<StringRope>());
					break;
				case StringRopeMain::StringBounce:
					strings.push_back(std::make_unique<StringBounce>());
					break;
				}

				strings.back()->SetStringLength(std::fabsf((float)(initMousePos.x - newMousePos.x)));
				strings.back()->SetPosition((sf::Vector2f)initMousePos);

				break;
			}
			break;
		}
	}

	void Render(sf::RenderWindow& window, int index) {
		if (isPressed) {
			currentMousePos = sf::Mouse::getPosition(window);

			auto [x1, y1] = (sf::Vector2f)initMousePos;
			auto [x2, y2] = (sf::Vector2f)currentMousePos;

			//For a straight horizontal line (Preview Render)
			DrawLine(window, x1, y1, x2, y1, index == 0 ? sf::Color::White : sf::Color::Magenta);
		}
	}
};

class Game {
private:
	sf::RenderWindow window;
	sf::Vector2u windowSize;
	
	sf::RectangleShape pixel, activeString;
	float pixelSize;

	LineEditor lineEditor;
	StringRopesVector strings;
	int activeStringIndex;

	bool isKeyPressed;

	Level level;

	Player player;

	void Input() {
		auto KeyPress = [](sf::Keyboard::Key key) {
			return sf::Keyboard::isKeyPressed(key);
		};

		auto MouseButton = [](sf::Mouse::Button button) {
			return sf::Mouse::isButtonPressed(button);
		};

		sf::Vector2i mousePos = sf::Mouse::getPosition(window);
		if (MouseButton(sf::Mouse::Right)) {

			auto [x, y] = sf::Vector2i(mousePos.x / pixelSize, mousePos.y / pixelSize);

			char c = isKeyPressed ? '.' : '#';
			level.SetCharacter((unsigned)x, (unsigned)y, c);
		}

		player.HorizontalMove((int)(KeyPress(sf::Keyboard::D) - KeyPress(sf::Keyboard::A)));
		for (auto& a : strings) {
			if (KeyPress(sf::Keyboard::W) && a->isPlayerOnString) {
				player.Jump();
			}
		}

		if (KeyPress(sf::Keyboard::W) && player.GetIsContact()) {
			player.GetIsContact() = false;
			player.Jump();
		}
	}

	void Logic() {
		player.Logic(level);
		for (auto& a : strings) {
			a->Logic(player);
		}
	}

	void Render() {
		for (uint32_t i = 0; i < level.GetHeight(); i++) {
			for (uint32_t j = 0; j < level.GetWidth(); j++) {
				switch (level.GetCharacter(j, i)) {
				case '#':
					pixel.setFillColor(sf::Color::Yellow);
					break;
				case '.':
					continue;
					break;
				}
			
				pixel.setPosition({ j * pixelSize, i * pixelSize });
				window.draw(pixel);
			}
		}

		activeString.setFillColor(activeStringIndex == 0 ? sf::Color::White : sf::Color::Magenta);z
		window.draw(activeString);

		lineEditor.Render(window, activeStringIndex);

		player.Render(window);
		for (auto& a : strings) {
			a->Render(window);
		}
	}

	void ManageEvent(sf::Event e) {
		switch (e.type) {
		case sf::Event::Closed:
			window.close();
			break;
		case sf::Event::MouseWheelScrolled:
			switch ((int)e.mouseWheelScroll.delta) {
			case -1:
				activeStringIndex = StringRopeMain::StringRope;
				break;
			case 1:
				activeStringIndex = StringRopeMain::StringBounce;
				break;
			}
			break;
		case sf::Event::KeyPressed:
			switch (e.key.code) {
			case sf::Keyboard::Z:
				if (strings.size() > 0) {
					strings.pop_back();
				}
				break;
			case sf::Keyboard::LShift:
				isKeyPressed = true;
				break;
			}
			break;
		case sf::Event::KeyReleased:
			switch (e.key.code) {
			case sf::Keyboard::LShift:
				isKeyPressed = false;
				break;
			}
			break;
		}

		lineEditor.ManageEvent(strings, activeStringIndex, e);
	}
public:
	Game(uint32_t x, uint32_t y, const sf::String& title)
		: windowSize(x, y),
		  window({ x, y }, title) {
		window.setFramerateLimit(60);

		pixelSize = 32.0f;
		player.SetPosition({ 32.0f, 32.0f });

		isKeyPressed = false;

		pixel.setSize({ pixelSize, pixelSize });
		activeString.setSize({ pixelSize, pixelSize });

		level.SetLevel({
			"################",
			"#..............#",
			"#..............#",
			"#..............#",
			"#..............#",
			"#..............#",
			"#..............#",
			"#..............#",
			"#..............#",
			"#..............#",
			"#..............#",
			"#####..........#",
			"#####..........#",
			"#########......#",
			"##########.....#",
			"################"
		});
	}

	void GameLogic() {
		while (window.isOpen()) {
			sf::Event e;
			while (window.pollEvent(e)) {
				ManageEvent(e);
			}
		
			Input();
			Logic();

			window.clear();
			Render();
			window.display();
		}
	}

	void Run() {
		GameLogic();
	}
};

int main() {

	Game game(512, 512, "Title");
	game.Run();

	return 0;
}