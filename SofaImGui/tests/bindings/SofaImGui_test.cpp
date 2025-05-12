#include <sofa/helper/Utils.h>

#include <sofa/testing/BaseTest.h>
using sofa::testing::BaseTest;

#include <sofa/simulation/graph/DAGSimulation.h>
#include <sofa/simulation/Node.h>
using sofa::simulation::Simulation ;
using sofa::simulation::Node ;

#include <sofa/simpleapi/SimpleApi.h>

namespace sofaimgui::bindings_test
{

    struct TestSofaImGuiBindings : public BaseTest
    {
        void doSetUp() override
        {
            sofa::simpleapi::importPlugin("SofaPython3");
        }

        bool initScene(std::string sceneName)
        {
            const std::string fileName = std::string(SOFAIMGUI_TEST_DIR) + "bindings/" + sceneName;
            auto root = sofa::core::objectmodel::SPtr_dynamic_cast<sofa::simulation::Node>(sofa::simulation::node::load(fileName.c_str()));

            sofa::simulation::node::initRoot(root.get());

            // Test if root is not null
            if(!root)
            {
                ADD_FAILURE() << "Error in init for the scene: " << sceneName << std::endl;
                return false;
            }

            return true;
        }
    };

    TEST_F(TestSofaImGuiBindings, initScene)
    {
        ASSERT_TRUE(this->initScene("test_imgui_bindings_scene.py"));
        ASSERT_NO_THROW(this->initScene("test_imgui_bindings_scene.py"));
    }

}
