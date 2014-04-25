//
//  Platform.h
//  levelBuilder
//
//  Created by Taylor Woods on 4/15/14.
//  Copyright (c) 2014 Taylor Woods. All rights reserved.
//

#ifndef __levelBuilder__Platform__
#define __levelBuilder__Platform__

#include <iostream>
#include "GameObject.h"
#include "glm/glm.hpp"
#include "GLHandles.h"

class Platform : public GameObject
{
   public:
      void step();
      void draw();
      Platform();
      Platform(glm::vec3 pos, GLHandles hand, GameModel model);
      Platform(glm::vec3 pos, glm::vec3 size, float rotation, GLHandles hand, GameModel model);
      bool detectCollision(glm::vec3 pos);
      glm::vec3 getPos();
      void setXPos(float val);
      void setYPos(float val);
      float getRot();
      void setRot(float val);
      glm::vec3 getSize();
      void stretch();
      void shrink();
      string toString();
      float rotation;
   private:
      void SetModel(glm::vec3 loc, glm::vec3 size, float rotation);
      GameModel mod;
};

#endif /* defined(__levelBuilder__Platform__) */
