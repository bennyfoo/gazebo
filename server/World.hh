/*
 * Copyright 2011 Nate Koenig & Andrew Howard
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
*/
/* Desc: The world; all models are collected here
 * Author: Andrew Howard and Nate Koenig
 * Date: 3 Apr 2007
 * SVN: $Id$
 */

#ifndef WORLD_HH
#define WORLD_HH

#include <iostream>
#include <vector>
#include <deque>
#include <map>
#include <string>


#include "Global.hh"
#include "Event.hh"
#include "Messages.hh"
#include "Vector3.hh"
#include "Pose3d.hh"
#include "Entity.hh"

namespace boost
{
  class recursive_mutex;
  class thread;
}

namespace libgazebo
{
  class Server;
}

namespace gazebo
{
/// \addtogroup gazebo_server
/// \brief The World
/// \{

// Forward declarations
  class Model;
  class Body;
  class Geom;
  class PhysicsEngine;
  class XMLConfigNode;
  //class OpenAL;
  class WorldState;
  class Time;
  class Scene;
  class FactoryIfaceHandler;
  class GraphicsIfaceHandler;
  class SimulationIfaceHandler;
   
/// \brief The World
/*
 * The world class keps a list of all models, handles loading and saving,
 * object dynamics and collision detection for contact joints
 */
class World 
{
  /// Private constructor
  public: World();

  /// Private destructor
  public: ~World();

  /// Load the world
  /// \param node XMLConfig node point
  public: void Load(XMLConfigNode *rootNode);//, unsigned int serverId);

  /// Save the world
  /// \param stream Output stream
  public: void Save(std::string &prefix, std::ostream &stream);

  /// \brief Initialize the world
  public: void Init();

  /// \briefRun the world in a thread
  public: void Start();

  /// \brief Stop the world
  public: void Stop();

  /// \brief Function to run physics. Used by physicsThread
  private: void RunLoop();

  /// \brief Update the world
  private: void Update();

  /// \brief Primarily used to update the graphics interfaces
  public: void GraphicsUpdate();

  /// Finilize the world
  public: void Fini();

  /// \brief Remove all entities from the world
  public: void Clear();

  /// \brief Get the name of the world
  public: std::string GetName() const;

  /// \brief Get the number of parameters
  public: unsigned int GetParamCount() const;

  /// \brief Get a param
  public: Param *GetParam(unsigned int index) const;

  /// Retun the libgazebo server
  /// \return Pointer the the libgazebo server
  public: libgazebo::Server *GetGzServer() const;

  /// Return the physics engine
  /// \return Pointer to the physics engine
  public: PhysicsEngine *GetPhysicsEngine() const;

  /// \brief Load all entities
  /// \param node XMLConfg node pointer
  /// \param parent Parent of the model to load
  /// \param removeDuplicate Remove existing model of same name
  public: void LoadEntities(XMLConfigNode *node, Common *parent, 
                            bool removeDuplicate,bool initModel);

  /// \brief Insert an entity into the world. This function pushes the model
  //  (encoded as an XML string) onto a list. The Graphics Thread will then
  //  call the ProcessEntitiesToLoad function to actually make the new
  //  entities. This Producer-Consumer model is necessary for thread safety.
  public: void InsertEntity(std::string xmlString);

  /// \brief Load all the entities that have been queued
  public: void ProcessEntitiesToLoad();

  /// \brief Delete all the entities that have been queued
  public: void ProcessEntitiesToDelete();

  /// \brief Get the number of models
  public: unsigned int GetModelCount() const;

  /// \brief Get a model based on an index
  public: Model *GetModel(unsigned int index);

  /// \brief Reset the simulation to the initial settings
  public: void Reset();

  /// \brief register a geom
  public: void RegisterGeom(Geom *geom);

  /// \brief Register a body 
  public: void RegisterBody(Body *body);

  /// \brief Goto a position in time
  //public: void GotoTime(double pos);

  /// \brief Get the selected entity
  public: Entity *GetSelectedEntity() const;

  /// \brief Print entity tree
  public: void PrintEntityTree();

  /// Get the simulation time
  /// \return The simulation time
  public: Time GetSimTime() const;

  /// \brief Set the sim time
  public: void SetSimTime(Time t);

  /// Get the pause time
  /// \return The pause time
  public: Time GetPauseTime() const;

  /// Get the start time
  /// \return The start time
  public: Time GetStartTime() const;

  /// Get the real time (elapsed time)
  /// \return The real time
  public: Time GetRealTime() const;

  /// \brief Returns the state of the simulation true if paused
  public: bool IsPaused() const;

  /// \brief Set whether the simulation is paused
  public: void SetPaused(bool p);

  /// \brief Get the scene 
  public: Scene *GetScene() const;

  /// \brief Get an element by name
  public: Common *GetByName(const std::string &name);

  /// \brief Receive a message
  public: void ReceiveMessage( const google::protobuf::Message &message );

  /// \brief Process all messages
  private: void ProcessMessages();

  /// \brief Save the state of the world
  private: void SaveState();

  /// \breif Set the state of the world to the pos pointed to by the iterator
  private: void SetState(std::deque<WorldState>::iterator iter);

  /// \brief Pause callback
  private: void PauseCB(bool p);

  /// \brief Step callback
  private: void StepCB();

  /// \brief Load a model
  /// \param node Pointer to the XMLConfig node
  /// \param parent The parent model
  /// \param removeDuplicate Remove existing model of same name
  /// \return The model that was created
  private: Model *LoadModel(XMLConfigNode *node, Common *parent, bool removeDuplicate,bool initModel);

  /// \brief Delete an entity by name
  /// \param name The name of the entity to delete
  private: void DeleteEntityCB(const std::string &name);

  /// \brief Set the selected entity
  private: void SetSelectedEntityCB( const std::string &name );

  /// Pointer the physics engine
  private: PhysicsEngine *physicsEngine;

  private: Common *rootElement;

  /// An abstract entity that is the root of the Entity Tree
  private: std::vector<Model*> models;

  /// List of models to delete from the world
  private: std::vector< std::string > toDeleteEntities;

  private: std::vector< std::string > toLoadEntities;

  /// Simulator control interface
  private: libgazebo::Server *server;

  /// thread in which the world is updated
  private: boost::thread *thread;

  private: bool stop;

  /// Interface handlers
  private: FactoryIfaceHandler *factoryIfaceHandler;
  private: GraphicsIfaceHandler *graphics;
  private: SimulationIfaceHandler *simIfaceHandler;

  //private: OpenAL *openAL;

  /// List of all the parameters
  protected: std::vector<Param*> parameters;

  /// The entity currently selected by the user
  private: Entity *selectedEntity;

  private: std::deque<WorldState> worldStates;
  private: std::deque<WorldState>::iterator worldStatesInsertIter;
  private: std::deque<WorldState>::iterator worldStatesEndIter;
  private: std::deque<WorldState>::iterator worldStatesCurrentIter;

  private: boost::recursive_mutex mutex;
  private: std::vector<google::protobuf::Message> messages;

  private: ParamT<std::string> *nameP;
  private: ParamT<Time> *saveStateTimeoutP;
  private: ParamT<unsigned int> *saveStateBufferSizeP;

  /// Current simulation time
  private: Time simTime, pauseTime, startTime;
  private: bool pause;
  private: bool stepInc;

  // Scene graph for the world
  private: Scene *scene;

  private: std::vector<ConnectionPtr> connections;
};

class WorldState
{
  public: std::map<std::string, Pose3d> modelPoses;
  public: std::map<std::string, Pose3d> bodyPoses;
  public: std::map<std::string, Pose3d> geomPoses;
};

/// \}
}

#endif
