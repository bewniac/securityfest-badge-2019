#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>

#define UP 0
#define MRIGHT 1
#define DOWN 2
#define MLEFT 3
#define STARTX 42
#define STARTY 24
#define MAXWIDTH 82
#define MAXHEIGHT 46

class Block {
  /*
   * Block defined by coordinates X and Y, and direction D.
   * D = 0 up, D = 1 MRIGHT, D = 2 down, D = 3 left.
   * 
   */
  private:
    int x, y;
    
  public:
    Block *next;
    Block() {
      this->x=0;
      this->y=0;
      this->next = NULL;
    }
    Block(int x, int y) {
      this->x = x;
      this->y = y;
      this->next = NULL;
    }
    
    void setX(int x) {
      this->x = x; 
    }
    
    void setY(int y) {
      this->y = y;
    }

    int getX() {
      return this->x;
    }
    
    int getY() {
      return this->y;
    }

    virtual ~Block() {};
};
class Food {
  private:
    Block *yummy;

  public:
    Food() {
      int x = random(2, MAXWIDTH);
      int y = random(2, MAXHEIGHT);
      if ((x % 2)==1) {
        x++;
      }
      if ((y % 2)==1) {
        y++;
      }
      yummy = new Block(x, y);
    }
    void newFood() {
      int x = random(2, MAXWIDTH);
      int y = random(2, MAXHEIGHT);
      if ((x % 2)==1) {
        x++;
      }
      if ((y % 2)==1) {
        y++;
      }
      this->yummy->setX(x);
      this->yummy->setY(y);
    }
    int getX() {
      return this->yummy->getX();
    }
    int getY() {
      return this->yummy->getY();
    }
    void DrawFood(Adafruit_PCD8544 &display) {
      display.drawRect(this->yummy->getX(), this->yummy->getY(), 2, 2, 1);
    }
};
class Snake {
 private:
  Block *head;
  int len;
  int dir;
  
 public:
  Snake() {
    this->head = new Block(STARTX, STARTY);
    this->head->next = new Block(STARTX-2, STARTY);
    this->head->next->next = new Block(STARTX-4, STARTY);
    this->len = 3;
    this->dir = MRIGHT;
  }
  
  Snake(int x, int y, int l) {
    this->head = new Block(x,y);
    this->len = l;
    this->dir = MRIGHT;
    Block *tmp = this->head;
    for (int i=0; i<this->len; i++) {
      tmp->next = new Block((x-2)-(i*2),y);
      tmp = tmp->next;
    }
    tmp->next = NULL;
  }
  int getX() {
    return this->head->getX();
  }
  int getY() {
    return this->head->getY();
  }
  int GetDirection() {
    return this->dir;
  }
  void SetDirection(int dir) {
    this->dir = dir;
  }
  
  void MoveSnake() {
    if (this->dir == UP) {
      Block *newHead = new Block(this->head->getX(), this->head->getY()-2);
      newHead->next = this->head;
      this->head = newHead;
      Block *tmp = this->head;
      for (int i=0; i<this->len; i++) {
        tmp = tmp->next;
      }
      tmp->next = NULL;
    }
    if (this->dir == DOWN) {
      Block *newHead = new Block(this->head->getX(), this->head->getY()+2);
      newHead->next = this->head;
      this->head = newHead;
      Block *tmp = this->head;
      for (int i=0; i<this->len; i++) {
        tmp = tmp->next;
      }
      tmp->next = NULL;
    }
     if (this->dir == MLEFT) {
      Block *newHead = new Block(this->head->getX()-2, this->head->getY());
      newHead->next = this->head;
      this->head = newHead;
      Block *tmp = this->head;
      for (int i=0; i<this->len; i++) {
        tmp = tmp->next;
      }
      tmp->next = NULL;
    }
   if (this->dir == MRIGHT) {
      Block *newHead = new Block(this->head->getX()+2, this->head->getY());
      newHead->next = this->head;
      this->head = newHead;
      Block *tmp = this->head;
      for (int i=0; i<this->len; i++) {
        tmp = tmp->next;
      }
      tmp->next = NULL;
    }
  }
  void addBlock() {
    Block *tmp = this->head;
    this->len++;
    bool isLast = false;
    while(isLast == false) {
      if (tmp->next == NULL) {
        isLast = true;
      }
      else {
        tmp = tmp->next;
      }
    }
    if (this->dir == MRIGHT) {
      tmp = new Block(tmp->getX()+2, tmp->getY());
    }
    if (this->dir == MLEFT) {
      tmp = new Block(tmp->getX()-2, tmp->getY());
    }
    if (this->dir == UP) {
      tmp = new Block(tmp->getX(), tmp->getY()-2);
    }
    if (this->dir == DOWN) {
      tmp = new Block(tmp->getX(), tmp->getY()+2);
    }
  }
  bool Collision() {
    if (this->head->getX() < 0) {
      return true;
    }
    if (this->head->getX() > MAXWIDTH) {
      return true;
    }
    if (this->head->getY() < 0) {
      return true;
    }
    if (this->head->getY() > MAXHEIGHT) {
      return true;
    }
    return false;
}
  void DrawSnake(Adafruit_PCD8544 &display) {
    Block *tmp = this->head;
    display.drawRect(tmp->getX(), tmp->getY(), 2, 2, 1);
    for (int i=0; i<this->len; i++) {
      tmp = tmp->next;
      display.drawRect(tmp->getX(), tmp->getY(), 2, 2, 1);
    }
  }
  
};

class Game {
  
  private:
    Snake *s;
    Food *f;
    int Button;
    
  public:
    Game(int Button) {
      Serial.begin(115200);
      this->Button = Button;
      pinMode(Button, INPUT);
      this->f = new Food();
      this->s = new Snake(STARTX, STARTY, 3);
    }
    void Run(Adafruit_PCD8544 &display) {
      while(1) {
        display.clearDisplay();
        int bPressed = analogRead(Button);
        if (400 < bPressed) { 
          if ( bPressed < 400 ) {  
            if (s->GetDirection() == UP) {
              s->SetDirection(MRIGHT);
            }
            else if (s->GetDirection() == MRIGHT) {
              s->SetDirection(DOWN);
            }
            else if (s->GetDirection() == DOWN) {
              s->SetDirection(MLEFT);
            }
            else if (s->GetDirection() == MLEFT) {
              s->SetDirection(UP);
            }
            delay(100);
          }
          if (bPressed < 400) { 
            if (s->GetDirection() == UP) {
              s->SetDirection(MLEFT);
            }
            else if (s->GetDirection() == MRIGHT) {
              s->SetDirection(UP);
            }
            else if (s->GetDirection() == DOWN) {
              s->SetDirection(MRIGHT);
            }
            else if (s->GetDirection() == MLEFT) {
              s->SetDirection(DOWN);
            }
            delay(100);
          }
        }
        if (this->s->getX() == this->f->getX()) {
          if (this->s->getY() == this->f->getY()) {
            this->f->newFood();
            this->s->addBlock();
          }
        }
        if (this->s->getY() == this->f->getY()) {
          if (this->s->getX() == this->f->getX()) {
            this->f->newFood();
            this->s->addBlock();
          }
        }
        if (this->s->Collision()==true) {
          display.clearDisplay();
          display.println("GAME OVER");
          display.display();
          return;
        }
        this->s->MoveSnake();
        this->s->DrawSnake(display);
        this->f->DrawFood(display);
        display.display();
        delay(100);
      }
    }
};
