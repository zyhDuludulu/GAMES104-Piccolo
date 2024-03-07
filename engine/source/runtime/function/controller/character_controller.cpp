#include "runtime/function/controller/character_controller.h"

#include "runtime/core/base/macro.h"

#include "runtime/function/framework/component/motor/motor_component.h"
#include "runtime/function/framework/world/world_manager.h"
#include "runtime/function/global/global_context.h"
#include "runtime/function/physics/physics_scene.h"

namespace Piccolo
{
    CharacterController::CharacterController(const Capsule& capsule) : m_capsule(capsule)
    {
        m_rigidbody_shape                                    = RigidBodyShape();
        m_rigidbody_shape.m_geometry                         = PICCOLO_REFLECTION_NEW(Capsule);
        *static_cast<Capsule*>(m_rigidbody_shape.m_geometry) = m_capsule;

        m_rigidbody_shape.m_type = RigidBodyShapeType::capsule;

        Quaternion orientation;
        orientation.fromAngleAxis(Radian(Degree(90.f)), Vector3::UNIT_X);

        m_rigidbody_shape.m_local_transform =
            Transform(Vector3(0, 0, capsule.m_half_height + capsule.m_radius), orientation, Vector3::UNIT_SCALE);
    }

    Vector3 CharacterController::move(const Vector3& current_position, const Vector3& displacement)
    {
        std::shared_ptr<PhysicsScene> physics_scene =
            g_runtime_global_context.m_world_manager->getCurrentActivePhysicsScene().lock();
        ASSERT(physics_scene);

        Vector3 final_position = current_position;

        Transform final_transform = Transform(final_position, Quaternion::IDENTITY, Vector3::UNIT_SCALE);

        std::vector<PhysicsHitInfo> hits;
        Vector3                     horizontal_displacement = Vector3(displacement.x, displacement.y, 0);
        Vector3                     vertical_displacement   = Vector3(0, 0, displacement.z);
        if (physics_scene->sweep(m_rigidbody_shape,
                                 final_transform.getMatrix(),
                                 horizontal_displacement.normalisedCopy(),
                                 horizontal_displacement.length(),
                                 hits))
        {
            Vector3 norm = hits[0].hit_normal.normalisedCopy();
            Vector3 movement = horizontal_displacement - norm.dotProduct(horizontal_displacement) * norm * 1.1; // multiply 1.001 for accuracy
            final_position += movement;
        }
        else
        {
            final_position += horizontal_displacement;
        }

        final_position += vertical_displacement;
        final_transform.m_position = final_position;

        if (physics_scene->isOverlap(m_rigidbody_shape, final_transform.getMatrix()))
        {
            final_position -= horizontal_displacement;
        }

        return final_position;
    }
} // namespace Piccolo
