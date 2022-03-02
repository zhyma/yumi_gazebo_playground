#include <ros/ros.h>
#include <gazebo/common/Plugin.hh>
#include <gazebo/physics/physics.hh>

// #include <functional>
#include <gazebo/gazebo.hh>
#include <gazebo/common/common.hh>
// #include <ignition/math/Pose3.hh>
// #include <ignition/math/Vector3.hh>
#include <gazebo/rendering/rendering.hh>

#include <iostream>

#include "ros/ros.h"
#include <yumi_gazebo/CylinderProperties.h>
#include "std_msgs/String.h"

#include <sys/time.h>

#define PUB_DATA      000
#define REMOVE_MODELS 100
#define CREATE_ROD    200
#define UPDATE_ROD    300
#define CREATE_CABLE  400

using namespace yumi_gazebo;

namespace gazebo
{
  class RLMod : public WorldPlugin
  {
    public:
      RLMod() : WorldPlugin()
      {
      }

      void Load(physics::WorldPtr _world, sdf::ElementPtr _sdf)
      {
        sdf = _sdf;
        // Make sure the ROS node for Gazebo has already been initialized
        if (!ros::isInitialized())
        {
          ROS_FATAL_STREAM("A ROS node for Gazebo has not been initialized, unable to load plugin. "
            << "Load the Gazebo system plugin 'libgazebo_ros_api_plugin.so' in the gazebo_ros package)");
          return;
        }

        ROS_INFO("Loading\n");

        // init ROS node
        int argc = 0;
        char** argv = NULL;
        ros::init(argc, argv, "Rod_properties_controller");
        // ROS publisher: pub rod position, orientation, and dimension
        properties_pub = n.advertise<CylinderProperties>("get_rod_properties", 1000);
        properties_sub = n.subscribe("set_rod_properties", 1000, &RLMod::properties_callback, this);

        this->node = transport::NodePtr(new gazebo::transport::Node());
        this->node->Init(_world->Name());
        visPub = this->node->Advertise<msgs::Visual>("~/visual");
        visPub->WaitForConnection();

        // // Create a publisher on the ~/factory topic
        factPub = node->Advertise<msgs::Factory>("~/factory");

        // loading models
        this->world = _world;
        this->add_entity_connection = event::Events::ConnectAddEntity(
                                      std::bind(&RLMod::addEntityEventCallback, 
                                      this, std::placeholders::_1));

        // Create the message
        msgs::Factory msg;

        // load rod
        msg.set_sdf_filename("model://rod");

        // Pose to initialize the model to
        msgs::Set(msg.mutable_pose(),
            ignition::math::Pose3d(
              ignition::math::Vector3d(0.2, 0.0, 0.2),
              ignition::math::Quaterniond(0, 0, 0)));

        // Send the message
        factPub->Publish(msg);

        // load cable
        msg.set_sdf_filename("model://cable");

        // Pose to initialize the model to
        msgs::Set(msg.mutable_pose(),
            ignition::math::Pose3d(
              // ignition::math::Vector3d(0.2-0.6, 0, 0.2+0.1),
              ignition::math::Vector3d(0.2-0.1, 0, 0.2+0.1),
              ignition::math::Quaterniond(0, 0, 0)));

        // Send the message
        factPub->Publish(msg);

        // _world->InsertModelFile("model://rod");
        // _world->InsertModelFile("model://cable");

        this->updateConnection = event::Events::ConnectWorldUpdateBegin(
          std::bind(&RLMod::OnUpdate, this));

      }
      void OnUpdate()
      {

        if (/*this->rod == NULL &&*/ rod_found==true)
        {
          // Get rod's pointer
          std::cout << "-> Get rod ptr" << std::endl;
          this->rod = this->world->ModelByName("rod");
          if (this->rod != NULL)
          {
            std::cout << "Get model: " << rod->GetName() << std::endl;
            rod_loaded = true;
          }
          this->rod_found = false;
        }
        if (/*this->cable==NULL &&*/ cable_found==true)
        {
          // Get cable's pointer
          std::cout << "-> Get cable ptr" << std::endl;
          this->cable = this->world->ModelByName("cable");
          if (this->cable != NULL)
          {
            std::cout << "Get model: " << cable->GetName() << std::endl;
            cable_loaded = true;
          }
          this->cable_found = false;
        }

        // environment update state machine
        if (this->stage == REMOVE_MODELS)
        {
          std::cout << "-> Remove models" << std::endl;
          if (this->cable != NULL && this->rod != NULL)
          {
            world->RemoveModel(this->cable->GetName()); 
            world->RemoveModel(this->rod->GetName()); 
            rod_loaded = false;
            cable_loaded = false;
            this->stage = CREATE_ROD;
          }
          this->prev_timestamp = get_timestamp();
        }
        else if (this->stage == CREATE_ROD)
        {
          // Delete model takes time. (Set to 500ms for now)
          long long now_timestamp = get_timestamp();
          if (now_timestamp-this->prev_timestamp > 1000)
          {
            std::cout << "-> Create rod" << std::endl;
            msgs::Factory msg;
            msg.set_sdf_filename("model://rod");

            // Pose to initialize the model to
            msgs::Set(msg.mutable_pose(),
                ignition::math::Pose3d(
                  ignition::math::Vector3d(properties[0], properties[1], properties[2]),
                  ignition::math::Quaterniond(0, 0, 0)));
            factPub->Publish(msg);

            this->stage = UPDATE_ROD;
          }
        }
        else if (this->stage == UPDATE_ROD)
        {
          if(rod_loaded)
          {
            std::cout << "-> Update rod position" << std::endl;
            physics::LinkPtr link = this->rod->GetLink("target_rod");

            // update rod's visual
            // Get link visual property
            msgs::Visual visualMsg = link->GetVisualMessage("target_rod_vis");
            // prepare visual message
            visualMsg.set_name(link->GetScopedName());
            visualMsg.set_parent_name(rod->GetScopedName());

            // update shape
            msgs::Geometry *geomMsg = visualMsg.mutable_geometry();
            geomMsg->set_type(msgs::Geometry::CYLINDER);
            geomMsg->mutable_cylinder()->set_radius(properties[3]);
            geomMsg->mutable_cylinder()->set_length(properties[4]);

            // get color
            visualMsg.mutable_material()->mutable_script()->set_name("Gazebo/Red");

            visPub->Publish(visualMsg);

            // Get link collision property
            physics::CollisionPtr collision = link->GetCollision("target_rod_collision");
            physics::ShapePtr shape = collision->GetShape();
            int shape_type = collision->GetShapeType();
            boost::shared_ptr<physics::CylinderShape> cylinder = boost::dynamic_pointer_cast<physics::CylinderShape>(shape);

            cylinder->SetSize(properties[3], properties[4]);
            this->stage = CREATE_CABLE;
          }
    
        }
        else if (this->stage == CREATE_CABLE)
        {
          std::cout << "-> Create cable" << std::endl;
          msgs::Factory msg;
          msg.set_sdf_filename("model://cable");

          // Pose to initialize the model to
          msgs::Set(msg.mutable_pose(),
              ignition::math::Pose3d(
                ignition::math::Vector3d(properties[0]-0.2, properties[1], properties[2]+properties[3]+0.03),
                ignition::math::Quaterniond(0, 0, 0)));
          factPub->Publish(msg);

          this->stage = PUB_DATA;
        }
        else if (this->stage == PUB_DATA)
        {
          if (rod_loaded)
          {
            // Get link "target_rod"
            physics::LinkPtr link = this->rod->GetLink("target_rod");
            // Get link collision property
            physics::CollisionPtr collision = link->GetCollision("target_rod_collision");
            physics::ShapePtr shape = collision->GetShape();
            int shape_type = collision->GetShapeType();
            boost::shared_ptr<physics::CylinderShape> cylinder = boost::dynamic_pointer_cast<physics::CylinderShape>(shape);

            // publish link pose and states
            CylinderProperties msg;
            ignition::math::Pose3d pose = link->WorldPose();
            // ignition::math::Vector3<double> position = pose.Pos();

            // x, y, z, radius, length
            msg.position.x = pose.Pos().X();
            msg.position.y = pose.Pos().Y();
            msg.position.z = pose.Pos().Z();
            msg.orientation.x = pose.Rot().X();
            msg.orientation.y = pose.Rot().Y();
            msg.orientation.z = pose.Rot().Z();
            msg.orientation.w = pose.Rot().W();
            msg.r = cylinder->GetRadius();
            msg.l = cylinder->GetLength();
            properties_pub.publish(msg);
            if (new_properties)
            {
              this->stage = REMOVE_MODELS;
              new_properties = false;
            }
          }
        }
        else
        {
          // error happens?
          if (this->cable != NULL && cable_found != false)
          {
            this->stage = PUB_DATA;
          }
        }
      }

    private:
      // ROS node part
      ros::NodeHandle n;
      ros::Publisher properties_pub;
      ros::Subscriber properties_sub;

      // Gazebo part
      sdf::ElementPtr sdf;

      physics::WorldPtr world;
      // Pointer to the update event connection
      physics::ModelPtr rod = NULL;
      physics::ModelPtr cable = NULL;
      bool rod_found = false;
      bool cable_found = false;
      bool rod_loaded = false;
      bool cable_loaded = false;
      // stage is either: PUB_DATA, REMOVE_CABLE, MOVE_ROD, or CREATE_CABLE
      int stage = PUB_DATA;
      // Pointer to the update event connection
      event::ConnectionPtr updateConnection;
      event::ConnectionPtr add_entity_connection;

      long long prev_timestamp = 0;

      // To update the visual
      transport::NodePtr node;
      transport::PublisherPtr visPub;
      transport::PublisherPtr factPub;

      bool new_properties;
      double properties[5];

      void addEntityEventCallback(const std::string &name)
      {
        // Check entity name...
        // Trigger initialization...
        std::cout << "Add Entity get:" << name << std::endl;
        if (name=="rod")
          rod_found = true;
        std::cout << "Add Entity get:" << name << std::endl;
        if (name=="cable")
          cable_found = true;
      }

      void properties_callback(const CylinderProperties::ConstPtr& msg)
      {
        this->properties[0] = msg->position.x;
        this->properties[1] = msg->position.y;
        this->properties[2] = msg->position.z;
        this->properties[3] = msg->r;
        this->properties[4] = msg->l;
        new_properties = true;
        std::cout << "subcriber callback get: ";
        std::cout << properties[0] << ", " << properties[1] << ", " << properties[2];
        std::cout << ", " << properties[3] << ", " << properties[4] << ", " <<std::endl;
      }

      long long get_timestamp()
      {
        struct timeval te;
        gettimeofday(&te, NULL); // get current time
        long long milliseconds = te.tv_sec*1000LL + te.tv_usec/1000; // calculate milliseconds
        // printf("milliseconds: %lld\n", milliseconds);
        return milliseconds;
      }

  };
  GZ_REGISTER_WORLD_PLUGIN(RLMod)
}
