#include <iostream>
#include <SFML/Graphics.hpp>
#define TINYC2_IMPL
#include <tinyc2.h>
#include <AnimatedSprite.h>
#include <GameObject.h>
#include <Player.h>
#include <NPC.h>
#include <Input.h>
#include <Debug.h>

using namespace std;

void updateBoundingBox(VertexArray& t_box, GameObject& t_player);

int main()
{
	enum ShapeControl { AABB, CIRCLE };
	ShapeControl currentShape = AABB;

	// Create the main window
	sf::RenderWindow window(sf::VideoMode(800, 600), "SFML window");

	// Load a NPC's sprites to display
	sf::Texture npc_texture;
	if (!npc_texture.loadFromFile("assets\\grid.png")) {
		DEBUG_MSG("Failed to load file");
		return EXIT_FAILURE;
	}

	// Load a mouse texture to display
	sf::Texture player_texture;
	if (!player_texture.loadFromFile("assets\\player.png")) {
		DEBUG_MSG("Failed to load file");
		return EXIT_FAILURE;
	}

	// Setup NPC's Default Animated Sprite
	AnimatedSprite npc_animated_sprite(npc_texture);
	npc_animated_sprite.addFrame(sf::IntRect(3, 3, 84, 84));
	npc_animated_sprite.addFrame(sf::IntRect(88, 3, 84, 84));
	npc_animated_sprite.addFrame(sf::IntRect(173, 3, 84, 84));
	npc_animated_sprite.addFrame(sf::IntRect(258, 3, 84, 84));
	npc_animated_sprite.addFrame(sf::IntRect(343, 3, 84, 84));
	npc_animated_sprite.addFrame(sf::IntRect(428, 3, 84, 84));

	// Setup Players Default Animated Sprite
	AnimatedSprite player_animated_sprite(player_texture);
	player_animated_sprite.addFrame(sf::IntRect(3, 3, 84, 84));
	player_animated_sprite.addFrame(sf::IntRect(88, 3, 84, 84));
	player_animated_sprite.addFrame(sf::IntRect(173, 3, 84, 84));
	player_animated_sprite.addFrame(sf::IntRect(258, 3, 84, 84));
	player_animated_sprite.addFrame(sf::IntRect(343, 3, 84, 84));
	player_animated_sprite.addFrame(sf::IntRect(428, 3, 84, 84));


	// Setup the NPC
	GameObject &npc = NPC(npc_animated_sprite);

	// Setup the Player
	GameObject &player = Player(player_animated_sprite);

	//Setup NPC AABB
	c2AABB aabb_npc;
	aabb_npc.min = c2V(npc.getAnimatedSprite().getPosition().x, npc.getAnimatedSprite().getPosition().y);
	aabb_npc.max = c2V(
		npc.getAnimatedSprite().getPosition().x +
		npc.getAnimatedSprite().getGlobalBounds().width, 
		npc.getAnimatedSprite().getPosition().y +
		npc.getAnimatedSprite().getGlobalBounds().height);

	//Setup Player AABB
	c2AABB aabb_player;
	aabb_player.min = c2V(player.getAnimatedSprite().getPosition().x, player.getAnimatedSprite().getPosition().y);
	aabb_player.max = c2V(player.getAnimatedSprite().getGlobalBounds().width / 6, player.getAnimatedSprite().getGlobalBounds().width / 6);

	// player circle
	sf::CircleShape playerCircle(50);

	// setup capsule colision
	sf::RectangleShape capsuleRect;
	capsuleRect.setSize(sf::Vector2f{ 100, 50 });
	capsuleRect.setPosition(sf::Vector2f{ 200, 200 });
	capsuleRect.setFillColor(sf::Color(sf::Color(255, 255, 255, 0)));
	capsuleRect.setOutlineThickness(1);
	//capsuleRect.setout
	sf::CircleShape capsuleCircles[2];

	for (int i = 0; i < 2; i++)
	{
		capsuleCircles[i].setRadius(25);
		capsuleCircles[i].setFillColor(sf::Color(255, 255, 255, 0));
		capsuleCircles[i].setOutlineThickness(1);
	}

	capsuleCircles[0].setPosition(sf::Vector2f{ 200 - capsuleCircles[0].getRadius(), 200 });
	capsuleCircles[1].setPosition(sf::Vector2f{ 300 - capsuleCircles[0].getRadius(), 200 });

	c2Capsule capsule;
	capsule.a = c2V(capsuleRect.getPosition().x, capsuleRect.getPosition().y + capsuleCircles[0].getRadius());
	capsule.b = c2V(capsuleRect.getPosition().x + 100, capsuleRect.getPosition().y + capsuleCircles[0].getRadius());
	capsule.r = 25;

	// setup polygon
	sf::CircleShape triangle(50, 3);
	triangle.setPosition(200, 400);
	triangle.setFillColor(sf::Color(255, 255, 255, 0));
	triangle.setOutlineThickness(1);

	c2Poly polygon;
	polygon.count = 3;
	polygon.verts[0] = c2V(triangle.getPoint(0).x + triangle.getPosition().x, triangle.getPoint(0).y + triangle.getPosition().y);
	polygon.verts[1] = c2V(triangle.getPoint(1).x + triangle.getPosition().x, triangle.getPoint(1).y + triangle.getPosition().y);
	polygon.verts[2] = c2V(triangle.getPoint(2).x + triangle.getPosition().x, triangle.getPoint(2).y + triangle.getPosition().y);
	c2MakePoly(&polygon);

	// setup ray
	
	sf::Vector2f rayPointOne{ 10, 10 };
	sf::Vector2f rayPointTwo{ 50, 200 };
	sf::Vertex ray[] =
	{
		sf::Vertex(rayPointOne),
		sf::Vertex(rayPointTwo)
	};

	c2Ray rayCollision;
	c2Raycast cast;

	rayCollision.p = c2V(rayPointOne.x, rayPointOne.y);

	sf::Vector2f distance = rayPointTwo - rayPointOne;
	float magnitude = sqrt((distance.x * distance.x) + (distance.y * distance.y));
	sf::Vector2f unitVector = distance / magnitude;

	rayCollision.d = c2Norm(c2V(unitVector.x, unitVector.y));
	rayCollision.t = magnitude;
	

	// setup bounding box
	sf::VertexArray boundingBox(sf::LinesStrip);
	

	// Initialize Input
	Input input;

	// Collision result
	int result = 0;

	// Direction of movement of NPC
	sf::Vector2f direction(0.1f, 0.2f);
	
	// Start the game loop
	while (window.isOpen())
	{
		// Move Sprite Follow Mouse

		player.getAnimatedSprite().setPosition(window.mapPixelToCoords(sf::Mouse::getPosition(window)));

		playerCircle.setPosition(window.mapPixelToCoords(sf::Mouse::getPosition(window)));

		// Move The NPC
		sf::Vector2f move_to(npc.getAnimatedSprite().getPosition().x + direction.x, npc.getAnimatedSprite().getPosition().y + direction.y);

		updateBoundingBox(boundingBox, player);

		if (move_to.x < 0) {
			direction.x *= -1;
			move_to.x = 0 + npc.getAnimatedSprite().getGlobalBounds().width;
		}
		else if (move_to.x + npc.getAnimatedSprite().getGlobalBounds().width >= 800) {
			direction.x *= -1;
			move_to.x = 800 - npc.getAnimatedSprite().getGlobalBounds().width;
		}
		else if (move_to.y < 0) {
			direction.y *= -1;
			move_to.y = 0 + npc.getAnimatedSprite().getGlobalBounds().height;
		}
		else if (move_to.y + npc.getAnimatedSprite().getGlobalBounds().height >= 600) {
			direction.y *= -1;
			move_to.y = 600 - npc.getAnimatedSprite().getGlobalBounds().height;
		}

		npc.getAnimatedSprite().setPosition(move_to);

		// Update NPC AABB set x and y
		aabb_npc.min = c2V(
			npc.getAnimatedSprite().getPosition().x,
			npc.getAnimatedSprite().getPosition().y
		);

		aabb_npc.max = c2V(
			npc.getAnimatedSprite().getPosition().x +
			npc.getAnimatedSprite().getGlobalBounds().width,
			npc.getAnimatedSprite().getPosition().y +
			npc.getAnimatedSprite().getGlobalBounds().height
		);

		// Update Player AABB
		aabb_player.min = c2V(
			player.getAnimatedSprite().getPosition().x,
			player.getAnimatedSprite().getPosition().y
		);
		aabb_player.max = c2V(
			player.getAnimatedSprite().getPosition().x +
			player.getAnimatedSprite().getGlobalBounds().width,
			player.getAnimatedSprite().getPosition().y +
			player.getAnimatedSprite().getGlobalBounds().height
		);

		// Process events
		sf::Event event;
		while (window.pollEvent(event))
		{
			switch (event.type)
			{
			case sf::Event::Closed:
				// Close window : exit
				window.close();
				break;
			case sf::Event::KeyPressed:
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
				{
					input.setCurrent(Input::Action::LEFT);
				}
				else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
				{
					input.setCurrent(Input::Action::RIGHT);
				}
				else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
				{
					input.setCurrent(Input::Action::UP);
				}
				else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num1))
				{
					currentShape = AABB;
				}
				else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num2))
				{
					currentShape = CIRCLE;
				}
				break;
			default:
				input.setCurrent(Input::Action::IDLE);
				break;
			}
		}

		// Handle input to Player
		player.handleInput(input);

		// Update the Player
		player.update();

		// Update the Player
		npc.update();

		// Check for collisions
		result = c2AABBtoAABB(aabb_player, aabb_npc);
		cout << ((result != 0) ? ("Collision") : "") << endl;

		if (result) 
		{
			player.getAnimatedSprite().setColor(sf::Color(255, 0, 0));

			for (int i = 0; i < boundingBox.getVertexCount(); i++)
			{
				boundingBox[i].color = sf::Color::Red;
			}
		}

		else
		{
			player.getAnimatedSprite().setColor(sf::Color(0, 255, 0));
		}

		result = 0;

		// collision: AABB->Capsule
		result = c2AABBtoCapsule(aabb_player, capsule);
		cout << ((result != 0) ? ("Collision capsule") : "") << endl;

		if (result)
		{
			for (int i = 0; i < 2; i++)
			{
				capsuleCircles[i].setOutlineColor(sf::Color::Red);
			}

			capsuleRect.setOutlineColor(sf::Color::Red);
			
		}

		else
		{
			for (int i = 0; i < 2; i++)
			{
				capsuleCircles[i].setOutlineColor(sf::Color::White);
			}

			capsuleRect.setOutlineColor(sf::Color::White);
		}

		result = 0;

		// collision: AABB->Poly
		result = c2AABBtoPoly(aabb_player, &polygon, NULL);
		cout << ((result != 0) ? ("Collision polygon") : "") << endl;

		if (result)
		{

			triangle.setOutlineColor(sf::Color::Red);

		}

		else
		{
			
			triangle.setOutlineColor(sf::Color::White);
		}

		result = 0;

		// collision: AABB->ray
		result = c2RaytoAABB(rayCollision, aabb_player, &cast);
		cout << ((result != 0) ? ("Collision ray") : "") << endl;

		if (result)
		{

			ray[0].color = sf::Color::Red;
			ray[1].color = sf::Color::Red;

		}

		else
		{

			ray[0].color = sf::Color::White;
			ray[1].color = sf::Color::White;
		}

		// Clear screen
		window.clear();

		// Draw the Players Current Animated Sprite
		//window.draw(player.getAnimatedSprite());

		// Draw the NPC's Current Animated Sprite
		window.draw(npc.getAnimatedSprite());

		window.draw(capsuleRect);

		for (int i = 0; i < 2; i++)
		{
			window.draw(capsuleCircles[i]);
		}

		window.draw(triangle);

		window.draw(ray, 2, sf::Lines);

		if (currentShape == AABB)
		window.draw(boundingBox);

		if (currentShape == CIRCLE)
		window.draw(playerCircle);

		// Update the window
		window.display();
	}

	return EXIT_SUCCESS;
};

void updateBoundingBox(VertexArray& t_box, GameObject& t_player)
{
	t_box.clear();

	t_box.append(sf::Vector2f{ t_player.getAnimatedSprite().getPosition().x, t_player.getAnimatedSprite().getPosition().y });

	t_box.append(sf::Vector2f{ t_player.getAnimatedSprite().getPosition().x + t_player.getAnimatedSprite().getGlobalBounds().width,
		t_player.getAnimatedSprite().getPosition().y });

	t_box.append(sf::Vector2f{ t_player.getAnimatedSprite().getPosition().x + t_player.getAnimatedSprite().getGlobalBounds().width,
		t_player.getAnimatedSprite().getPosition().y });

	t_box.append(sf::Vector2f{ t_player.getAnimatedSprite().getPosition().x + t_player.getAnimatedSprite().getGlobalBounds().width,
		t_player.getAnimatedSprite().getPosition().y + t_player.getAnimatedSprite().getGlobalBounds().width });

	t_box.append(sf::Vector2f{ t_player.getAnimatedSprite().getPosition().x + t_player.getAnimatedSprite().getGlobalBounds().width,
			t_player.getAnimatedSprite().getPosition().y + t_player.getAnimatedSprite().getGlobalBounds().width });

	t_box.append(sf::Vector2f{ t_player.getAnimatedSprite().getPosition().x,
			t_player.getAnimatedSprite().getPosition().y + t_player.getAnimatedSprite().getGlobalBounds().width });

	t_box.append(sf::Vector2f{ t_player.getAnimatedSprite().getPosition().x,
			t_player.getAnimatedSprite().getPosition().y + t_player.getAnimatedSprite().getGlobalBounds().width });

	t_box.append(sf::Vector2f{ t_player.getAnimatedSprite().getPosition().x, t_player.getAnimatedSprite().getPosition().y });
}