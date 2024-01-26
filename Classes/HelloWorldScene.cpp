/****************************************************************************
 Copyright (c) 2017-2018 Xiamen Yaji Software Co., Ltd.
 
 http://www.cocos2d-x.org
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/

#include "HelloWorldScene.h"
#include "SimpleAudioEngine.h"
#include "ui/CocosGUI.h"

#include <vector>
#include <ctime>
#include <time.h>
#include <iostream>
#include <memory>

USING_NS_CC;

using namespace ui;
using namespace std;

int countColor = 3;

enum colors_list { //min - 3, max - 8
    red = 1,
    green = 2,
    blue = 3,
    yellow = 4,
    orange = 5,
    pink = 6, // розовый (177, 41, 173)
    turquoise = 7, //бирюзовый (37, 227, 252)
    grey = 8
};

Color3B change_color(int max) {
    auto res = std::rand() % max + 1;

    switch (res)
    {
    case colors_list::red:
        return Color3B::RED;
    case colors_list::green:
        return Color3B::GREEN;
    case colors_list::blue:
        return Color3B::BLUE;
    case colors_list::yellow:
        return Color3B::YELLOW;
    case colors_list::orange:
        return Color3B::ORANGE;
    case colors_list::pink:
        return Color3B(177, 41, 173);
    case colors_list::turquoise:
        return Color3B(37, 227, 252);
    case colors_list::grey:
        return Color3B::GRAY;
    default:
        return Color3B::WHITE;
    }
};

auto myMove = MoveBy::create (5, Vec2(50, 0));

class ColoredTile {
    Button* button;

    static int combo;
    static int score;
public:
    //соседние клетки
    ColoredTile* left_nhbr;
    ColoredTile* right_nhbr;
    ColoredTile* up_nhbr;
    ColoredTile* down_nhbr;

    bool to_delete; //флаг того, что тайл подготовлен к удалению

    pair<int, int> position; // позиция в сетке

    static int color_count; // кол-во возможных цветов будет задаваться вне 

    ColoredTile(int x, int y) {
        to_delete = false;
        position = pair<int, int>(x, y); // назначим позицию 
        button = Button::create("Tile.png");
        
        button->setColor(change_color(color_count));

        button->addTouchEventListener([&](Ref* sender, Widget::TouchEventType type) {
            switch (type)
            {
            case ui::Widget::TouchEventType::BEGAN:
                break;
            case ui::Widget::TouchEventType::ENDED:
                button->runAction(myMove);
                button->update(NULL);
                break;
            default:
                break;
            }
            });
    }

    
    Button* getButton_p() {
        return button;
    }

    void changePosition(pair <int, int>& next_position) {
        position = next_position;
    }

    static void set_color_count(int count) {
        color_count = count;
    }

    void set_left_neigbour(ColoredTile* nhbr) {
        left_nhbr = nhbr;
    }
    void set_right_neigbour(ColoredTile* nhbr) {
        right_nhbr = nhbr;
    }
    void set_up_neigbour(ColoredTile* nhbr) {
        up_nhbr = nhbr;
    }
    void set_down_neigbour(ColoredTile* nhbr) {
        down_nhbr = nhbr;
    }
    
    void touchEvent(ColoredTile* sender) {
        combo++;

        if (sender == nullptr) {
            return;
        }
        else if (this->button->getColor() == sender->button->getColor()) { // проверка нужна для первого срабатывания

            to_delete = true;
            

            if (!left_nhbr->to_delete &&
                this->button->getColor() == left_nhbr->button->getColor())  // эстафета передается только если совпадает цвет плитка не была ранее проверена
            {
                left_nhbr->touchEvent(this);
            }
            if (!right_nhbr->to_delete &&
                this->button->getColor() == right_nhbr->button->getColor()) {
                right_nhbr->touchEvent(this);
            }
            if (!up_nhbr->to_delete &&
                this->button->getColor() == up_nhbr->button->getColor()) {
                up_nhbr->touchEvent(this);
            }
            if (!down_nhbr->to_delete &&
                this->button->getColor() == down_nhbr->button->getColor()) {
                down_nhbr->touchEvent(this);
            }



            if (combo >= 3) {
                button->setColor(Color3B::WHITE);
                button->update(NULL);
            }

        }

    }
};

int ColoredTile::color_count = 0;
int ColoredTile::combo = 0;
int ColoredTile::score = 0;

vector<vector<ColoredTile*>> web; // хранилище всех экземпляров

void check();
    
void generate_tiles(int x, int y, HelloWorld* scene, Vec2& sceneSize) {
    //отступ между плитками
    int tab_between_tile = 5;
    // определение начала игрового поля
    Vec2 game_area_begin(215, 187);
    //пока так для полного заполнения площади
    x = 20;
    y = 15;

    int step = 45; // 40 + 5 

    for (int column_number = 0; column_number < x; column_number++) {
        vector<ColoredTile*> buffer;
        for (int string_number = 0; string_number < y; string_number++) {
            auto block = new ColoredTile(column_number, string_number);

            scene->addChild(block->getButton_p());
            block->getButton_p()->setPosition(Vec2(game_area_begin.x + column_number * step, game_area_begin.y + string_number * step));

            buffer.push_back(block);
        }
        web.push_back(buffer);
    }
    
    //когда всё поле проинициализированно - присваиваем соседей
    for (int column_number = 0; column_number < x; column_number++) {
        for (int string_number = 0; string_number < y; string_number++) 
        {
            ColoredTile* tile = web[column_number][string_number];
            // присвоим левого соседа
            if (column_number > 0) {
                tile->set_left_neigbour(web[column_number-1][string_number]);
            }
            else {
                tile->set_left_neigbour(nullptr);
            }
            //присвоим правого соседа
            if (column_number < x-1) {
                tile->set_right_neigbour(web[column_number+1][string_number]);
            }
            else {
                tile->set_right_neigbour(nullptr);
            }
            //присвоим верхнего соседа
            if (string_number < y - 1) {
                tile->set_up_neigbour(web[column_number][string_number+1]);
            }
            else {
                tile->set_up_neigbour(nullptr);
            }
            //присвоим нижнего соседа
            if (string_number > 0) {
                tile->set_down_neigbour(web[column_number][string_number - 1]);
            }
            else {
                tile->set_down_neigbour(nullptr);
            }
        }
    }
    //соседи присвоены

    
}


Scene* HelloWorld::createScene()
{
    return HelloWorld::create();
}

// Print useful error message instead of segfaulting when files are not there.
static void problemLoading(const char* filename)
{
    printf("Error while loading: %s\n", filename);
    printf("Depending on how you compiled you might have to add 'Resources/' in front of filenames in HelloWorldScene.cpp\n");
}

// on "init" you need to initialize your instance
bool HelloWorld::init()
{
    //////////////////////////////
    // 1. super init first
    if ( !Scene::init() )
    {
        return false;
    }

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    /////////////////////////////
    // 2. add a menu item with "X" image, which is clicked to quit the program
    //    you may modify it.

    // add a "close" icon to exit the progress. it's an autorelease object
    auto closeItem = MenuItemImage::create(
                                           "CloseNormal.png",
                                           "CloseSelected.png",
                                           CC_CALLBACK_1(HelloWorld::menuCloseCallback, this));

    if (closeItem == nullptr ||
        closeItem->getContentSize().width <= 0 ||
        closeItem->getContentSize().height <= 0)
    {
        problemLoading("'CloseNormal.png' and 'CloseSelected.png'");
    }
    else
    {
        float x = origin.x + visibleSize.width - closeItem->getContentSize().width/2;
        float y = origin.y + closeItem->getContentSize().height/2;
        closeItem->setPosition(Vec2(x,y));
    }

    // create menu, it's an autorelease object
    auto menu = Menu::create(closeItem, NULL);
    menu->setPosition(Vec2::ZERO);
    this->addChild(menu, 1);

    /////////////////////////////
    // 3. add your codes below...

    // add a label shows "Hello World"
    // create and initialize a label

    auto label = Label::createWithTTF("Hello World", "fonts/Marker Felt.ttf", 24);
    if (label == nullptr)
    {
        problemLoading("'fonts/Marker Felt.ttf'");
    }
    else
    {
        // position the label on the center of the screen
        label->setPosition(Vec2(origin.x + visibleSize.width/2,
                                origin.y + visibleSize.height - label->getContentSize().height));

        // add the label as a child to this layer
        this->addChild(label, 1);
    }

   /*
   * auto sprite = Sprite::create("HelloWorld.png");
    if (sprite == nullptr)
    {
        problemLoading("'HelloWorld.png'");
    }
    else
    {
        // position the sprite on the center of the screen
        sprite->setPosition(Vec2(visibleSize.width/2 + origin.x, visibleSize.height/2 + origin.y));

        // add the sprite as a child to this layer
        this->addChild(sprite, 0);
    }
   */


    ColoredTile::set_color_count(4);

    auto fon = Sprite::create("MyScenePicture.png");
    if (fon == nullptr)
    {
        problemLoading("'MyScenePicture.png'");
    }
    else
    {
        // position the sprite on the center of the screen
        fon->setPosition(Vec2(visibleSize.width / 2 + origin.x, visibleSize.height / 2 + origin.y));

        // add the sprite as a child to this layer
        this->addChild(fon, 0);
    }
    
    generate_tiles(16, 10, this, origin); //int x, int y, HelloWorld* scene, Vec2 &
   

    return true;
}


void HelloWorld::menuCloseCallback(Ref* pSender)
{
    //Close the cocos2d-x game scene and quit the application
    Director::getInstance()->end();

    /*To navigate back to native iOS screen(if present) without quitting the application  ,do not use Director::getInstance()->end() as given above,instead trigger a custom event created in RootViewController.mm as below*/

    //EventCustom customEndEvent("game_scene_close_event");
    //_eventDispatcher->dispatchEvent(&customEndEvent);


}
