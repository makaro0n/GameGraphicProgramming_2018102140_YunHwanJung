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
      Class:    OrbitCube

      Summary:  OrbitCube class

      Methods:  Update
                  Update the cube each frame 
                OrbitCube
                  Constructor.
                ~OrbitCube
                  Destructor.
C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C---C-C*/

class OrbitCube : public BaseCube
{
public:
    OrbitCube(const std::filesystem::path& textureFilePath);
    OrbitCube(const OrbitCube& other) = delete;
    OrbitCube(OrbitCube&& other) = delete;
    OrbitCube& operator=(const OrbitCube& other) = delete;
    OrbitCube& operator=(OrbitCube&& other) = delete;
    ~OrbitCube() = default;

    virtual void Update(_In_ FLOAT deltaTime) override;
};