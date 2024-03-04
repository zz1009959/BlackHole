#version 460 core

const float PI = 3.14159265359;
const float EPSILON = 0.0001;
const float INFINITY = 1000000.0;

out vec4 FragColor;

uniform vec2 resolution; // 视口分辨率（像素）
//uniform vec3 cameraPos; // 相机位置
uniform float time; // 时间
//uniform mat4 view; // 相机的视图矩阵

uniform float mouseX;
uniform float mouseY;
uniform float frontView = 0.0; // 前视角标志
uniform float topView = 0.0; // 顶视角标志
uniform float cameraRoll = 0.0; // 相机旋转角度

uniform float gravatationalLensing = 1.0; // 引力透镜效应标志
uniform float renderBlackHole = 1.0; // 渲染黑洞标志
uniform float mouseControl = 1.0; // 鼠标控制标志
uniform float fovScale = 1.0; // 视场缩放因子

struct Sphere
{
    vec3 center; // 球心
    float radius; // 半径
};

uniform samplerCube skybox; // 天空盒纹理

vec3 rotateVectorUsingQuaternion(vec3 position, vec3 axis, float angle)
{
    // 根据轴和角度创建四元数
    float half_angle = (angle * 0.5) * PI / 180.0;
    vec4 qr = vec4(axis * sin(half_angle), cos(half_angle));

    // 计算四元数的共轭
    vec4 qr_conj = vec4(-qr.xyz, qr.w);

    // 将位置向量转换为四元数形式
    vec4 q_pos = vec4(position, 0.0);

    // 四元数乘法
    vec4 q_tmp = vec4(
        qr.w * q_pos.x + qr.y * q_pos.z - qr.z * q_pos.y,
        qr.w * q_pos.y + qr.z * q_pos.x - qr.x * q_pos.z,
        qr.w * q_pos.z + qr.x * q_pos.y - qr.y * q_pos.x,
        -qr.x * q_pos.x - qr.y * q_pos.y - qr.z * q_pos.z
    );

    // 再次乘以四元数的共轭
    vec4 result = vec4(
        q_tmp.x * qr_conj.w + q_tmp.w * qr_conj.x + q_tmp.y * qr_conj.z - q_tmp.z * qr_conj.y,
        q_tmp.y * qr_conj.w + q_tmp.w * qr_conj.y + q_tmp.z * qr_conj.x - q_tmp.x * qr_conj.z,
        q_tmp.z * qr_conj.w + q_tmp.w * qr_conj.z + q_tmp.x * qr_conj.y - q_tmp.y * qr_conj.x,
        q_tmp.w * qr_conj.w - q_tmp.x * qr_conj.x - q_tmp.y * qr_conj.y - q_tmp.z * qr_conj.z
    );

    // 返回结果向量
    return result.xyz;
}

mat3 lookAt(vec3 origin, vec3 target, float roll)
{
    vec3 rr = vec3(sin(roll), cos(roll), 0.0);// 计算观察方向的右向量，即观察者位置到目标位置的方向
    vec3 ww = normalize(target - origin);// 计算观察方向的正向向量
    vec3 uu = normalize(cross(ww, rr));// 计算观察方向的上向量，通过叉乘得到
    vec3 vv = normalize(cross(uu, ww));// 计算观察方向的侧向向量，通过叉乘得到
    return mat3(uu, vv, ww);
}

// 判断光线和球体是否相交，并计算相交点的参数 t
bool raySphereIntersect(vec3 rayOrigin, vec3 rayDir, Sphere sphere, out float t)
{
    vec3 oc = rayOrigin - sphere.center;
    float a = dot(rayDir, rayDir);
    float b = 2.0 * dot(oc, rayDir);
    float c = dot(oc, oc) - sphere.radius * sphere.radius;
    float discriminant = b * b - 4.0 * a * c;

    if (discriminant < 0.0)
    {
        return false; // 没有相交
    }
    else
    {
        float t0 = (-b - sqrt(discriminant)) / (2.0 * a);
        float t1 = (-b + sqrt(discriminant)) / (2.0 * a);
        t = min(t0, t1);
        return true; // 存在相交点
    }
}

void main()
{
    mat3 view; // 视图矩阵
    vec3 cameraPos; // 相机位置

    vec2 mouse = clamp(vec2(mouseX, mouseY) / resolution.xy, 0.0, 1.0) - 0.5;
    cameraPos = vec3(-cos(mouse.x * 10.0) * 15.0, mouse.y * 30.0,sin(mouse.x * 10.0) * 15.0);

    vec3 target = vec3(0.0, 0.0, 0.0); // 目标位置为原点
    view = lookAt(cameraPos, target, radians(cameraRoll)); // 计算视图矩阵

    vec2 uv = gl_FragCoord.xy / resolution.xy - vec2(0.5); // 计算像素点坐标
    uv.x *= resolution.x / resolution.y; // 根据屏幕宽高比调整 x 坐标

    vec3 dir = normalize(vec3(-uv.x * fovScale, uv.y * fovScale, 1.0)); // 计算视线方向
    vec3 pos = cameraPos; // 设置初始位置为相机位置
    dir = view * dir; // 应用视图矩阵到视线方向

    // 定义球体
    Sphere sphere;
    sphere.center = vec3(0.0, 0.0, 0.0); // 球心位于 (0.0, 0.0, -5.0)
    sphere.radius = 1.0; // 半径为 1.0

    // 判断光线和球体是否相交
    float t;
    if (raySphereIntersect(cameraPos, dir, sphere, t))
    {
        FragColor = vec4(1.0, 1.0, 1.0, 1.0); // 如果光线和球体相交，用黑色表示
    }
    else
    {
        // 如果光线和球体不相交，计算天空盒的颜色
        dir = rotateVectorUsingQuaternion(dir, vec3(0.0, 1.0, 0.0), time);
        vec3 skyboxColor = texture(skybox, dir).rgb;
        FragColor = vec4(skyboxColor, 1.0);
    }
}