/*+===================================================================
  File:      ORBITCUBE.H

  Summary:  Child Cube header file contains declarations of
             OrbitCube class used for the lab samples of Game
             Graphics Programming course.

  Classes: Renderable

  � 2022 Kyung Hee University
===================================================================+*/
#pragma once

#include "Common.h"

#include "Cube/BaseCube.h"


/*C+C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C+++C
      Class:    CustomCube

      Summary:  CustomCube class

      Methods:  Update
                  Update the cube each frame
                CustomCube
                  Constructor.
                ~CustomCube
                  Destructor.
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/

class CustomCube : public BaseCube
{
public:
    CustomCube() = default;
    CustomCube(const CustomCube& other) = delete;
    CustomCube(CustomCube&& other) = delete;
    CustomCube& operator=(const CustomCube& other) = delete;
    CustomCube& operator=(CustomCube&& other) = delete;
    ~CustomCube() = default;

    virtual void Update(_In_ FLOAT deltaTime) override;

private:
    FLOAT m_deltaTime;
};