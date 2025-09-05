#pragma once

/*
 *  Copyright(c) 2020 Jeremiah van Oosten
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files(the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions :
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 *  IN THE SOFTWARE.
 */

 /**
  *  @file Light.h
  *  @date November 10, 2020
  *  @author Jeremiah van Oosten
  *
  *  @brief Light structures that use HLSL constant buffer padding rules.
  */

#include <DirectXMath.h>
#include <SimpleMath.h>

struct PointLight
{
    PointLight()
        : PositionWS(0.0f, 0.0f, 0.0f, 1.0f)
        , PositionVS(0.0f, 0.0f, 0.0f, 1.0f)
        , Color(1.0f, 1.0f, 1.0f, 1.0f)
        , Ambient(0.01f)
        , ConstantAttenuation(0.0f)
        , LinearAttenuation(1.0f)
        , QuadraticAttenuation(0.0f)
        , Radius(1.f)
    {
    }

    DirectX::SimpleMath::Vector4 PositionWS;  // Light position in world space.
    //----------------------------------- (16 byte boundary)
    DirectX::SimpleMath::Vector4 PositionVS;  // Light position in view space.
    //----------------------------------- (16 byte boundary)
    DirectX::SimpleMath::Vector4 Color;
    //----------------------------------- (16 byte boundary)
    float Ambient;
    float ConstantAttenuation;
    float LinearAttenuation;
    float QuadraticAttenuation;
    //----------------------------------- (16 byte boundary)
    float Radius;
    float Padding[3];
    //----------------------------------- (16 byte boundary)
    // Total:                              16 * 5 = 80 bytes

    void SetAttenuation(float constant= 1.f, float linear = 1.f, float quadratic = 0.f, float threshold = 0.01f)
    {
        Radius = CalculateLightRadius(constant, linear, quadratic, threshold);
    }

private:
    float CalculateLightRadius(float c, float l, float q, float threshold = 0.01f) {
        if (threshold <= 0.0f) return 0.0f; // Invalid threshold

        if (q == 0.0f) {
            if (l == 0.0f) {
                return c > 0.0f && (1.0f / c) >= threshold ? FLT_MAX : 0.0f;
            }
            float d = (1.0f / threshold - c) / l;
            return d > 0.0f ? d : 0.0f;
        }

        float a = q;
        float b = l;
        float c_coeff = c - 1.0f / threshold;
        float discriminant = b * b - 4.0f * a * c_coeff;

        if (discriminant < 0.0f) return FLT_MAX; // Light never attenuates below threshold

        float d = (-b + sqrtf(discriminant)) / (2.0f * a);
        return d > 0.0f ? d : 0.0f;
    }
};

struct SpotLight
{
    SpotLight()
        : PositionWS(0.0f, 0.0f, 0.0f, 1.0f)
        , PositionVS(0.0f, 0.0f, 0.0f, 1.0f)
        , DirectionWS(0.0f, 0.0f, 1.0f, 0.0f)
        , DirectionVS(0.0f, 0.0f, 1.0f, 0.0f)
        , Color(1.0f, 1.0f, 1.0f, 1.0f)
        , Ambient(0.01f)
        , SpotAngle(DirectX::XM_PIDIV2)
        , ConstantAttenuation(1.0f)
        , LinearAttenuation(0.0f)
        , QuadraticAttenuation(0.0f)
        , Radius(10.0f)
    {
    }

    DirectX::SimpleMath::Vector4 PositionWS;  // Light position in world space.
    //----------------------------------- (16 byte boundary)
    DirectX::SimpleMath::Vector4 PositionVS;  // Light position in view space.
    //----------------------------------- (16 byte boundary)
    DirectX::SimpleMath::Vector4 DirectionWS;  // Light direction in world space.
    //----------------------------------- (16 byte boundary)
    DirectX::SimpleMath::Vector4 DirectionVS;  // Light direction in view space.
    //----------------------------------- (16 byte boundary)
    DirectX::SimpleMath::Vector4 Color;
    //----------------------------------- (16 byte boundary)
    float Ambient;
    float SpotAngle;
    float ConstantAttenuation;
    float LinearAttenuation;
    //----------------------------------- (16 byte boundary)
    float QuadraticAttenuation;
    float Radius;
    float Padding[2];
    //----------------------------------- (16 byte boundary)
    // Total:                              16 * 7 = 112 bytes
};

struct DirectionalLight
{
    DirectionalLight()
        : DirectionWS(0.0f, 0.0f, 1.0f, 0.0f)
        , DirectionVS(0.0f, 0.0f, 1.0f, 0.0f)
        , Color(1.0f, 1.0f, 1.0f, 1.0f)
        , Ambient(0.1f)
    {
    }

    DirectX::SimpleMath::Vector4 DirectionWS;  // Light direction in world space.
    //----------------------------------- (16 byte boundary)
    DirectX::SimpleMath::Vector4 DirectionVS;  // Light direction in view space.
    //----------------------------------- (16 byte boundary)
    DirectX::SimpleMath::Vector4 Color;
    //----------------------------------- (16 byte boundary)
    float Ambient;
    float Padding[3];
    //----------------------------------- (16 byte boundary)
    // Total:                              16 * 4 = 64 bytes
};
