
#ifndef MYOPENPROJECT_GLOBALS_H
#define MYOPENPROJECT_GLOBALS_H

namespace Globals
{
    static constexpr unsigned int SCREEN_WIDTH{800};
    static constexpr unsigned int SCREEN_HEIGHT{600};

    static constexpr int SAMPLE_NUMBER{4};

    static float FOV{60.0f};
    static constexpr float MIN_FOV{10.0f};
    static constexpr float MAX_FOV{60.0f};

    static bool firstLook{true};
    static float lastX{static_cast<float>(SCREEN_WIDTH) / 2.0f};
    static float lastY{static_cast<float>(SCREEN_HEIGHT) / 2.0f};

    static bool hasFlashed{false};
    static bool closeFlash{false};

    static bool blinnPhong{false};
    static bool hasChangedLight{false};

}

#endif //MYOPENPROJECT_GLOBALS_H