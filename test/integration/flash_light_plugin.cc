/*
 * Copyright (C) 2018 Open Source Robotics Foundation
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

#include <string>
#include <sstream>
#include <cmath>

#include "gazebo/test/ServerFixture.hh"

using namespace gazebo;
class FlashLightPluginTest : public ServerFixture
{
};

// information to record for the lights in the enviornment
struct RecordInfo
{
  double duration;
  double interval;
  common::Time lastUpdate;
};

struct RecordInfo flashLight[4];
bool called;

void InitRec()
{
  common::Time currentTime = physics::get_world()->SimTime();
  flashLight[0].duration = -1;
  flashLight[1].duration = -1;
  flashLight[2].duration = -1;
  flashLight[3].duration = -1;
  flashLight[0].interval = -1;
  flashLight[1].interval = -1;
  flashLight[2].interval = -1;
  flashLight[3].interval = -1;
  flashLight[0].lastUpdate = currentTime;
  flashLight[1].lastUpdate = currentTime;
  flashLight[2].lastUpdate = currentTime;
  flashLight[3].lastUpdate = currentTime;
}

//////////////////////////////////////////////////
// Callback for light/modify topic
void lightCb(ConstLightPtr &_msg)
{
  // Determine which light is to be updated
  int indx;
  std::string name = _msg->name();
  std::stringstream ss;
  ss << name.substr(name.length() - 1);
  ss >> indx;
  indx--;

  bool indexInRange = true;
  if (indx < 0 || 3 < indx)
  {
    indexInRange = false;
  }
  EXPECT_TRUE(indexInRange);

  // Get the current time
  common::Time currentTime = physics::get_world()->SimTime();

  // Update to flash
  if (_msg->range() > 0)
  {
    flashLight[indx].interval
      = currentTime.Double() - flashLight[indx].lastUpdate.Double();
  }
  // Update to dim
  else
  {
    flashLight[indx].duration
      = currentTime.Double() - flashLight[indx].lastUpdate.Double();
  }

  // Update the last update time
  flashLight[indx].lastUpdate = currentTime;

  called = true;
}

//////////////////////////////////////////////////
TEST_F(FlashLightPluginTest, Blinking)
{
  this->Load("worlds/flash_light_plugin_demo.world", true);
  auto world = physics::get_world();
  ASSERT_NE(nullptr, world);

  // Get models
  auto model = world->ModelByName("light_model");
  ASSERT_NE(nullptr, model);

  // Initialize the time in the records
  InitRec();

  // Subscribe to plugin notifications
  called = false;
  transport::SubscriberPtr sceneSub
    = this->node->Subscribe("~/light/modify", &lightCb);

  // Let the plugin blink the lights for a while
  common::Time sTime = world->SimTime();
  flashLight[0].lastUpdate = sTime;
  flashLight[1].lastUpdate = sTime;
  flashLight[2].lastUpdate = sTime;
  flashLight[3].lastUpdate = sTime;
  world->Step(5000);
  common::Time eTime = world->SimTime();
  double lastUpdate0 = flashLight[0].lastUpdate.Double();
  double lastUpdate1 = flashLight[1].lastUpdate.Double();
  double lastUpdate2 = flashLight[2].lastUpdate.Double();
  double lastUpdate3 = flashLight[3].lastUpdate.Double();

  // Make sure if the function was called
  EXPECT_TRUE(called);

  // Verify only the supposed lights are updated
  // NOTE: Taking some errors caused by callback functions into consideration.
  //       Here, it is considered to be passed if the error is less than or
  //       equal to 0.01 sec.
  // NOTE: The first and second must have been updated within their phases.
  EXPECT_TRUE(fabs(lastUpdate0 - eTime.Double()) <= 0.41);
  EXPECT_TRUE(fabs(lastUpdate1 - eTime.Double()) <= 0.06);
  // NOTE: It is supposed to stop updating the third and forth lights just
  //       after the beginning.
  EXPECT_TRUE(fabs(lastUpdate2 - sTime.Double()) <= 0.01);
  EXPECT_TRUE(fabs(lastUpdate3 - sTime.Double()) <= 0.01);

  // Verify the lights blinking at the supposed duration and interval.
  // NOTE: Taking some errors caused by callback functions into consideration.
  //       Here, it is considered to be passed if the error is less than or
  //       equal to 0.01 sec.
  EXPECT_TRUE(fabs(flashLight[0].duration - 0.1) <= 0.01);
  EXPECT_TRUE(fabs(flashLight[0].interval - 0.4) <= 0.01);
  EXPECT_TRUE(fabs(flashLight[1].duration - 0.05) <= 0.01);
  EXPECT_TRUE(fabs(flashLight[1].interval - 0.05) <= 0.01);
}

//////////////////////////////////////////////////
int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}