#pragma warning( disable : 4244 )  // Disable dumb-dumb c4244

#include <SFML/Graphics.hpp> //sfml ftw
#include <SFML/Audio.hpp> // being able to hear ftw
#include <iostream> // console
#include <string> // for turning variables into strings that can be displayed
#include <cmath> // for complex maths like sqrt
#include <time.h> // time for setting srand
#include <stdlib.h> // srand, rand
#include "Collision.hpp" // working collision

struct entity {
    float x{};
    float y{};
    float velx{};
    float vely{};
    int rot{};
    float health{};
    float shield{};
    int ammo[2];
    int maxAmmo[2];
    bool thrust{};
    sf::Sprite sprite_ship;
    sf::Sprite sprite_exst;
    sf::Sound snd_engine;
};

float degRad(float degree) {
    return degree * 2.f * acos(0.f) / 180.f;
}

int main() {
    // settings
    sf::ContextSettings settings;
    settings.antialiasingLevel = 0;

    int vidWidth = 1920;
    int vidHeight = 1080;

    // create window
	sf::RenderWindow window(sf::VideoMode(vidWidth, vidHeight), "Booster 2", sf::Style::Fullscreen, settings);

    //window settings
	window.setFramerateLimit(144);
    window.setMouseCursorVisible(false);

    //      Setup

    const int spriteSize = 200;
    const int maxSpeed = 15.f;
    const int maxHealth = 100;
    const int maxShield = 200;

    int tick = 0;
    int targetRot = 0;
    int rotDir = 0;

    bool gamepad = false;

    struct entity player;
    player.health = maxHealth;
    player.shield = maxShield;
    player.maxAmmo[0] = 100;
    player.maxAmmo[1] = 20;
    player.ammo[0] = player.maxAmmo[0];
    player.ammo[1] = player.maxAmmo[1];

    // Assets
    
    // Images

    // set icon
    sf::Image icon;
    if (!icon.loadFromFile("res/img/booster-icon.png")) {
        std::cout << "Load Failed: res/img/booster-icon.png" << std::endl;
        system("pause");
    }

    window.setIcon(128, 128, icon.getPixelsPtr());

    // set background texture
    sf::Texture background;
    if (!background.loadFromFile("res/img/background.png")) {
        std::cout << "Load Failed: res/img/background.png" << std::endl;
        system("pause");
    }
    background.setRepeated(true);

    // set sprite texture
    sf::Texture spriteSheet;
    if (!spriteSheet.loadFromFile("res/img/thruster-sheet.png")) {
        std::cout << "Load Failed: res/img/thruster-sheet.png" << std::endl;
        system("pause");
    }
    spriteSheet.setSmooth(false);
    spriteSheet.setRepeated(false);

    // set scanlines texture
    sf::Texture scanTexture;
    if (!scanTexture.loadFromFile("res/img/scanlines.png")) {
        std::cout << "Load Failed: res/img/scanlines.png" << std::endl;
        system("pause");
    }
    scanTexture.setRepeated(true);
    scanTexture.setSmooth(false);

    // set vignette texture
    sf::Texture vigTexture;
    if (!vigTexture.loadFromFile("res/img/vignette.png")) {
        std::cout << "Load Failed: res/img/vignette.png" << std::endl;
        system("pause");
    }

    // Sprites

    // background sprite
    sf::Sprite bkg;
    bkg.setTextureRect(sf::IntRect(0, 0, vidWidth * 3, vidHeight * 3));
    bkg.setTexture(background); 

    // scanlines sprite
    sf::Sprite scanlines;
    scanlines.setColor(sf::Color(255, 255, 255, 50));
    scanlines.setTexture(scanTexture);
    scanlines.setTextureRect(sf::IntRect(0, 0, vidWidth, vidHeight));

    // vignette sprite
    sf::Sprite vignette;
    vignette.setTexture(vigTexture);
    vignette.setColor(sf::Color(255, 255, 255, 200));

    // set ship sprite
    player.sprite_ship.setTexture(spriteSheet);
    player.sprite_ship.setTextureRect(sf::IntRect(0, 0, 200, 400));
    player.sprite_ship.setScale(.25f, .25f);
    player.sprite_ship.setOrigin(spriteSize / 2, 200);
    player.sprite_ship.setPosition(vidWidth / 2.f, vidHeight / 2.f);

    // set exaust sprite
    player.sprite_exst.setTexture(spriteSheet);
    player.sprite_exst.setTextureRect(sf::IntRect(0, 400, 200, 398));
    player.sprite_exst.setScale(.35f, .35f);
    player.sprite_exst.setOrigin(spriteSize / 2, 10.f);
    player.sprite_exst.setPosition(vidWidth / 2.f, vidHeight / 2.f);

    // Status text sprite
    sf::Sprite hud_status;
    hud_status.setTexture(spriteSheet);
    hud_status.setTextureRect(sf::IntRect(spriteSize * 2, 0, 153, 28));
    hud_status.setPosition(20, 62);

    // current weapon sprite
    sf::Sprite hud_weapon;
    hud_weapon.setTexture(spriteSheet);
    hud_weapon.setTextureRect(sf::IntRect(400, 66, 148, 28));
    hud_weapon.setPosition(1755, 1040);

    // target rotation sprite
    sf::Sprite rotIndicator;
    rotIndicator.setTexture(spriteSheet);
    rotIndicator.setTextureRect(sf::IntRect(0, 800, 100, 100));
    rotIndicator.setOrigin(50, 100);
    rotIndicator.setPosition(vidWidth / 2, vidHeight / 2);
    rotIndicator.setColor(sf::Color(255, 255, 255, 100));
    rotIndicator.setScale(0.5, 0.5);

    // reticule sprite
    sf::Sprite reticule;
    reticule.setTexture(spriteSheet);
    reticule.setTextureRect(sf::IntRect(100, 800, 100, 100));
    reticule.setOrigin(50, 50);
    reticule.setScale(.4, .4);

    // shield bar
    sf::ConvexShape shieldBar;
    shieldBar.setPointCount(4);
    shieldBar.setPoint(0, sf::Vector2f(0, 0));
    shieldBar.setPoint(1, sf::Vector2f(0, 40));
    shieldBar.setPoint(2, sf::Vector2f(360, 40));
    shieldBar.setPoint(3, sf::Vector2f(400, 0));
    shieldBar.setPosition(20, 20);
    shieldBar.setFillColor(sf::Color(54, 90, 224));

    // health bar
    sf::ConvexShape healthBar;
    healthBar.setPointCount(4);
    healthBar.setPoint(0, sf::Vector2f(0, 0));
    healthBar.setPoint(1, sf::Vector2f(0, 13));
    healthBar.setPoint(2, sf::Vector2f(178, 13));
    healthBar.setPoint(3, sf::Vector2f(191, 0));
    healthBar.setPosition(180, 69);
    healthBar.setFillColor(sf::Color(56, 192, 50));

    // full health line
    sf::ConvexShape fillLine;
    fillLine.setPointCount(4);
    fillLine.setPoint(0, sf::Vector2f(0, 0));
    fillLine.setPoint(1, sf::Vector2f(-70, 70));
    fillLine.setPoint(2, sf::Vector2f(-66, 70));
    fillLine.setPoint(3, sf::Vector2f(4, 0));
    fillLine.setPosition(420, 20);
    fillLine.setFillColor(sf::Color(200, 200, 200, 255));

    // Sounds

    // set sound buffer for engine
    sf::SoundBuffer engineBuffer;
    if (!engineBuffer.loadFromFile("res/sounds/rocket-sound.wav")) {
        std::cout << "Load Failed: res/sounds/rocket-sound.wav" << std::endl;
        system("pause");
    }

    player.snd_engine.setBuffer(engineBuffer);
    player.snd_engine.setVolume(0);
    player.snd_engine.setLoop(true);
    player.snd_engine.play();

    // Music

    // set menu theme
    sf::Music theme;
    if (!theme.openFromFile("res/soundtrack/Booster.wav")) {
        std::cout << "Load Failed: res/soundtrack/Booster.wav" << std::endl;
        system("pause");
    }

    theme.setLoop(true);
    theme.setLoopPoints(sf::Music::TimeSpan(sf::milliseconds(8728), sf::milliseconds(139651 - 8728)));
    theme.play();

    // set font
    sf::Font font;
    if (!font.loadFromFile("res/fonts/kremlin.ttf")) {
        std::cout << "Load Failed: res/fonts/kremlin.ttf" << std::endl;
        system("pause");
    }

    // set text
    sf::Text debugText;
    debugText.setCharacterSize(32);
    debugText.setFont(font);
    debugText.setFillColor(sf::Color::White);
    debugText.setPosition(10, 500);

    std::string debugString;

    // set ammo remaining
    sf::Text ammoRemain;
    ammoRemain.setCharacterSize(48);
    ammoRemain.setFont(font);
    ammoRemain.setFillColor(sf::Color::White);
    ammoRemain.setPosition(1755, 984);

    // set ammo total
    sf::Text ammoTotal;
    ammoTotal.setCharacterSize(32);
    ammoTotal.setFont(font);
    ammoTotal.setFillColor(sf::Color(90, 90, 90, 150));
    ammoTotal.setPosition(1842, 1000);

    // menu stuff

    // set booster logo for menu
    sf::Text BoosterLogo;
    BoosterLogo.setCharacterSize(250);
    BoosterLogo.setFont(font);
    BoosterLogo.setPosition(367, vidHeight / 2 - 300);
    BoosterLogo.setString("BOOSTER");

    // set menu instructions
    sf::Text instText;
    instText.setCharacterSize(50);
    instText.setFont(font);
    instText.setPosition(710, vidHeight / 2 + 300);
    instText.setString("PRESS       TO START");

    // set button sprite for menu
    sf::Sprite aButton;
    aButton.setTexture(spriteSheet);
    aButton.setTextureRect(sf::IntRect(200, 800, 25, 25));
    aButton.setPosition(894, vidHeight / 2 + 305);
    aButton.setScale(2, 2);

    //      THE LOOP

    while (!sf::Keyboard::isKeyPressed(sf::Keyboard::Space) && !sf::Joystick::isButtonPressed(0, 0)) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape)) {
            return 0;
        }

        bkg.setPosition(fmod(bkg.getPosition().x + 1, vidWidth) - vidWidth, fmod(bkg.getPosition().y + 1, vidHeight) - vidHeight);

        window.clear();

        window.draw(bkg);
        window.draw(vignette);
        window.draw(BoosterLogo);
        window.draw(instText);
        window.draw(aButton);
        window.draw(scanlines);

        window.display();
    }

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        tick++;

        // Input

        // quit
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape)) {
            return 0;
        }

        // target rotation calculation

        // I am not sure why this worked but I am happy.

        // gamepad rotation
        if (gamepad) {
            if (std::abs(sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::X)) > 15 || std::abs(sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::Y)) > 15) {
                targetRot = (90.f / acos(0.f)) * atan2(sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::Y), sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::X));
                rotIndicator.setColor(sf::Color(255, 255, 255, 100));
            }
            else {
                rotIndicator.setColor(sf::Color(255, 255, 255, 0));
                targetRot = player.rot - 90;
            }
        }
        // mouse rotation
        else {
            targetRot = (90.f / acos(0.f)) * atan2(sf::Mouse::getPosition().y - vidHeight / 2, sf::Mouse::getPosition().x - vidWidth / 2); // get angle to mouse
        }

        if (targetRot < 0) { targetRot += 360; } // un-screw-up the atan
        targetRot = (targetRot + 90) % 360; // convert to sfml standard (0 at the top)

        // set the rotation indicator to the... do i need to explain this?
        rotIndicator.setRotation(targetRot);

        // find direction to rotate
        rotDir = player.rot - targetRot;
        if (rotDir > 180) {
            rotDir -= 360;
        }
        else if (rotDir < -180) {
            rotDir += 360;
        }

        // change rotation
        if (rotDir < -2) {
            player.rot += 2;
        }
        else if (rotDir > 2) {
            player.rot -= 2;
        }

        if (player.rot < 0) { player.rot += 360; }
        player.rot = player.rot % 360;

        // thrust
        if (sf::Joystick::getAxisPosition(0, sf::Joystick::Z) > 10 || sf::Keyboard::isKeyPressed(sf::Keyboard::W) || sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
            player.velx += std::cos(degRad(player.rot - 90)) / 50.f;
            player.vely += std::sin(degRad(player.rot - 90)) / 50.f;

            if (player.snd_engine.getVolume() < 50) { player.snd_engine.setVolume(player.snd_engine.getVolume() + 2); }
            else { player.snd_engine.setVolume(50); }
            player.thrust = true;
        }
        else { 
            if (player.snd_engine.getVolume() > 2) { player.snd_engine.setVolume(player.snd_engine.getVolume() - 2); }
            else { player.snd_engine.setVolume(0); }
            player.thrust = false; 
        }

        if (sf::Joystick::getAxisPosition(0, sf::Joystick::Z) < -10 || sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
            if (tick % 5 == 0 && player.ammo[0] > 0) { 
                player.ammo[0]--; 
            }
        }

        // Logic

        player.x += player.velx;
        player.y += player.vely;

        // speed limit
        if (player.vely > maxSpeed) { player.vely = maxSpeed; }
        if (player.vely < -maxSpeed) { player.vely = -maxSpeed; }
        if (player.velx > maxSpeed) { player.velx = maxSpeed; }
        if (player.velx < -maxSpeed) { player.velx = -maxSpeed; }

        player.sprite_ship.setRotation(player.rot);
        player.sprite_exst.setRotation(player.rot);

        bkg.setPosition(fmod(-player.x, vidWidth) - vidWidth, fmod(-player.y, vidHeight) - vidHeight);

        // update healthbars
        shieldBar.setPoint(2, sf::Vector2f(player.shield / maxShield * 400.f - 40.f, 40.f));
        shieldBar.setPoint(3, sf::Vector2f(player.shield / maxShield * 400.f, 0.f));

        healthBar.setPoint(2, sf::Vector2f(player.health / maxHealth * 191.f - 13.f, 13.f));
        healthBar.setPoint(3, sf::Vector2f(player.health / maxHealth * 191.f, 0.f));

        // update cursor position
        reticule.setPosition(sf::Mouse::getPosition().x, sf::Mouse::getPosition().y);

        // Draw

        window.clear();

        // draw background
        window.draw(bkg);

        // draw ship
        window.draw(player.sprite_ship);

        if (player.thrust) { window.draw(player.sprite_exst); }

        // draw vignette
        window.draw(vignette);

        // HUD

        // debug string
        debugString = std::to_string(player.rot) + "\n\n" + std::to_string(targetRot) + "\n\n" + std::to_string(rotDir);
        debugText.setString(debugString);
        // window.draw(debugText);

        window.draw(shieldBar);
        window.draw(healthBar);
        window.draw(hud_status);
        window.draw(fillLine);
        window.draw(hud_weapon);
        window.draw(rotIndicator);

        if (!gamepad) { window.draw(reticule); }

        ammoTotal.setString(std::to_string(player.maxAmmo[0]));
        window.draw(ammoTotal);

        if (player.ammo[0] == 0) { ammoRemain.setFillColor(sf::Color(120, 120, 120)); }
        ammoRemain.setString(std::to_string(player.ammo[0]));
        window.draw(ammoRemain);

        // draw scanlines
        window.draw(scanlines);

        // display frame
        window.display();
    }

	return 0;
}