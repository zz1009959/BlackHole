#version 460 core

const float PI = 3.14159265359;
const float EPSILON = 0.0001;
const float INFINITY = 1000000.0;

out vec4 FragColor;

uniform vec2 resolution; // 视口分辨率（像素）
uniform float time; // 时间

uniform float mouseX;
uniform float mouseY;
uniform float frontView = 0.0; // 前视角标志
uniform float topView = 0.0; // 顶视角标志
uniform float cameraRoll = 0.0; // 相机旋转角度

uniform float gravatationalLensing = 1.0; // 引力透镜效应标志
uniform float renderBlackHole = 1.0; // 渲染黑洞标志
uniform float mouseControl = 1.0; // 鼠标控制标志
uniform float fovScale = 1.0; // 视场缩放因子

uniform float adiskEnabled = 1.0; // 是否启用星盘效果，1.0 为启用，0.0 为禁用
uniform float adiskParticle = 1.0; // 是否使用颗粒效果，1.0 为使用，0.0 为不使用
uniform float adiskHeight = 0.2; // 星盘高度
uniform float adiskLit = 0.5; // 星盘光照强度
uniform float adiskDensityV = 1.0; // 垂直方向上的星盘密度
uniform float adiskDensityH = 1.0; // 水平方向上的星盘密度
uniform float adiskNoiseScale = 1.0; // 星盘噪声比例
uniform float adiskNoiseLOD = 5.0; // 星盘噪声 LOD（细节级别）
uniform float adiskSpeed = 0.5; // 星盘旋转速度

uniform samplerCube galaxy; // 天空盒纹理
uniform sampler2D colorMap; // 天空盒纹理

struct Sphere
{
    vec3 center; // 球心
    float radius; // 半径
};

struct Ring
{
    vec3 center;
    vec3 normal;
    float innerRadius;
    float outerRadius;
    float rotateSpeed;
};

///----
/// Simplex 3D Noise
/// by Ian McEwan, Ashima Arts

// 使用置换函数对向量进行置换
vec4 permute(vec4 x)
{
    return mod(((x * 34.0) + 1.0) * x, 289.0);
}

// 使用泰勒展开逆平方根函数对向量进行操作
vec4 taylorInvSqrt(vec4 r)
{
    return 1.79284291400159 - 0.85373472095314 * r;
}

float snoise(vec3 v)
{
  const vec2 C = vec2(1.0 / 6.0, 1.0 / 3.0); // 常数用于计算梯度
  const vec4 D = vec4(0.0, 0.5, 1.0, 2.0);   // 用于生成噪声的常数

  // First corner
  vec3 i = floor(v + dot(v, C.yyy));
  vec3 x0 = v - i + dot(i, C.xxx);

  // Other corners
  vec3 g = step(x0.yzx, x0.xyz);
  vec3 l = 1.0 - g;
  vec3 i1 = min(g.xyz, l.zxy);
  vec3 i2 = max(g.xyz, l.zxy);

  // 计算偏移向量
  //  x0 = x0 - 0. + 0.0 * C
  vec3 x1 = x0 - i1 + 1.0 * C.xxx;
  vec3 x2 = x0 - i2 + 2.0 * C.xxx;
  vec3 x3 = x0 - 1. + 3.0 * C.xxx;

  // Permutations（排列）
  i = mod(i, 289.0);
  vec4 p = permute(permute(permute(i.z + vec4(0.0, i1.z, i2.z, 1.0)) + i.y +
                           vec4(0.0, i1.y, i2.y, 1.0)) +
                   i.x + vec4(0.0, i1.x, i2.x, 1.0));

  // Gradients
  // 计算梯度
  // ( N*N points uniformly over a square, mapped onto an octahedron.)
  float n_ = 1.0 / 7.0; // N=7
  vec3 ns = n_ * D.wyz - D.xzx;

  vec4 j = p - 49.0 * floor(p * ns.z * ns.z); //  mod(p,N*N)

  vec4 x_ = floor(j * ns.z);
  vec4 y_ = floor(j - 7.0 * x_); // mod(j,N)

  vec4 x = x_ * ns.x + ns.yyyy;
  vec4 y = y_ * ns.x + ns.yyyy;
  vec4 h = 1.0 - abs(x) - abs(y);

  vec4 b0 = vec4(x.xy, y.xy);
  vec4 b1 = vec4(x.zw, y.zw);

  vec4 s0 = floor(b0) * 2.0 + 1.0;
  vec4 s1 = floor(b1) * 2.0 + 1.0;
  vec4 sh = -step(h, vec4(0.0));

  vec4 a0 = b0.xzyw + s0.xzyw * sh.xxyy;
  vec4 a1 = b1.xzyw + s1.xzyw * sh.zzww;

  vec3 p0 = vec3(a0.xy, h.x);
  vec3 p1 = vec3(a0.zw, h.y);
  vec3 p2 = vec3(a1.xy, h.z);
  vec3 p3 = vec3(a1.zw, h.w);

  // Normalise gradients
  vec4 norm =
      taylorInvSqrt(vec4(dot(p0, p0), dot(p1, p1), dot(p2, p2), dot(p3, p3)));
  p0 *= norm.x;
  p1 *= norm.y;
  p2 *= norm.z;
  p3 *= norm.w;

  // Mix final noise value
  vec4 m =
      max(0.6 - vec4(dot(x0, x0), dot(x1, x1), dot(x2, x2), dot(x3, x3)), 0.0);
  m = m * m;
  return 42.0 *
         dot(m * m, vec4(dot(p0, x0), dot(p1, x1), dot(p2, x2), dot(p3, x3)));
}
///----

// 计算射线与环结相交的距离
float ringDistance(vec3 rayOrigin, vec3 rayDir, Ring ring)
{
  float denominator = dot(rayDir, ring.normal);
  float constant = -dot(ring.center, ring.normal);

  // 如果射线方向与法线平行（或几乎平行）
  if (abs(denominator) < EPSILON)
  {
    return -1.0;
  }
  else
  {
    // 计算射线与环的交点参数t
    float t = -(dot(rayOrigin, ring.normal) + constant) / denominator;
    if (t < 0.0)
    {
      return -1.0;
    }

    vec3 intersection = rayOrigin + t * rayDir;

    // 计算交点到环心的距离
    float d = length(intersection - ring.center);
    if (d >= ring.innerRadius && d <= ring.outerRadius)
    {
      return t;
    }
    return -1.0;
  }
}

vec3 accel(float h2, vec3 pos)
{
  float r2 = dot(pos, pos);
  float r5 = pow(r2, 2.5);
  vec3 acc = -1.5 * h2 * pos / r5 * 1.0;
  return acc;
}

// 四元数旋转
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
// 罗德里格斯公式
vec3 rotateVectorUsingRodrigues(vec3 position, vec3 axis, float angle)
{
    // 将角度转换为弧度
    float radians = angle * PI / 180.0;

    // 计算旋转后的向量
    vec3 rotatedVector = position * cos(radians) +
                         cross(axis, position) * sin(radians) +
                         axis * dot(axis, position) * (1.0 - cos(radians));

    return rotatedVector;
}

void ringColor(vec3 rayOrigin, vec3 rayDir, Ring ring, inout float minDistance,
               inout vec3 color)
{
  // 计算射线与环的交点距离
  float distance = ringDistance(rayOrigin, normalize(rayDir), ring);
  // 如果交点距离在合理范围内
  if (distance >= EPSILON && distance < minDistance && distance <= length(rayDir) + EPSILON)
  {
    // 更新最小距离
    minDistance = distance;

    // 计算交点坐标
    vec3 intersection = rayOrigin + normalize(rayDir) * minDistance;
    vec3 ringColor;

    // 计算环的颜色
    {
      // 计算交点到原点的距离
      float dist = length(intersection);

      // 计算环上的垂直参数v
      float v = clamp((dist - ring.innerRadius) /
                          (ring.outerRadius - ring.innerRadius),
                      0.0, 1.0);

      // 计算环上的水平参数u
      vec3 base = cross(ring.normal, vec3(0.0, 0.0, 1.0));
      float angle = acos(dot(normalize(base), normalize(intersection)));
      if (dot(cross(base, intersection), ring.normal) < 0.0)
        angle = -angle;

      float u = 0.5 - 0.5 * angle / PI;
      // HACK
      // 水平参数u随时间变化
      u += time * ring.rotateSpeed;

      // 设置环的基础颜色
      vec3 color = vec3(0.0, 0.5, 1.0);
      // HACK
      // 设置透明度
      float alpha = 0.5;
      ringColor = vec3(color);
    }

    // 将环的颜色加到最终颜色上
    color += ringColor;
  }
}

// 根据方向向量获取全景图中的颜色
vec3 panoramaColor(sampler2D tex, vec3 dir)
{
  // 计算球坐标系下的纹理坐标
    vec2 uv = vec2(0.5 - atan(dir.z, dir.x) / PI * 0.5, 0.5 - asin(dir.y) / PI);
    // 使用球坐标系下的纹理坐标获取纹理颜色并返回
    return texture2D(tex, uv).rgb;
}

float sqrLength(vec3 a)
{
    return dot(a, a);
}

vec3 toSpherical(vec3 p)
{

  float rho = sqrt((p.x * p.x) + (p.y * p.y) + (p.z * p.z));// 计算球坐标中的径向距离 rho
  float theta = atan(p.z, p.x);// 计算球坐标中的方位角 theta
  float phi = asin(p.y / rho);// 计算球坐标中的极角 phi

  return vec3(rho, theta, phi);// 返回球坐标
}

void adiskColor(vec3 pos, inout vec3 color, inout float alpha)
{
    float innerRadius = 2.6;
    float outerRadius = 12.0;

    // 根据到黑洞中心的距离线性减小密度
    float density = max(0.0, 1.0 - length(pos.xyz / vec3(outerRadius, adiskHeight, outerRadius)));
    if (density < 0.001)
    {
        return;
    }

    density *= pow(1.0 - abs(pos.y) / adiskHeight, adiskDensityV);

    // 当半径小于内稳定圆轨道时，将粒子密度设置为0
    density *= smoothstep(innerRadius, innerRadius * 1.1, length(pos));

    // 当密度很小时，退出函数
    if (density < 0.001)
    {
        return;
    }

    vec3 sphericalCoord = toSpherical(pos);

    // 缩放 rho 和 phi，以便粒子在视觉上显示为正确的比例
    sphericalCoord.y *= 2.0;
    sphericalCoord.z *= 4.0;

    // 根据距离调整粒子密度
    density *= 1.0 / pow(sphericalCoord.x, adiskDensityH);
    density *= 16000.0;

    if (adiskParticle < 0.5)
    {
        color += vec3(0.0, 1.0, 0.0) * density * 0.02;
        return;
    }

    float noise = 1.0;
    for (int i = 0; i < int(adiskNoiseLOD); i++)
    {
        noise *= 0.5 * snoise(sphericalCoord * pow(i, 2) * adiskNoiseScale) + 0.5;
        if (i % 2 == 0)
        {
            sphericalCoord.y += time * adiskSpeed;
        }
        else
        {
            sphericalCoord.y -= time * adiskSpeed;
        }
    }

    // 获取尘埃颜色
    vec3 dustColor = texture(colorMap, vec2(sphericalCoord.x / outerRadius, 0.5)).rgb;

    // 计算最终颜色
    color += density * adiskLit * dustColor * alpha * abs(noise);
}

vec3 traceColor(vec3 pos, vec3 dir)
{
    // 初始化颜色和透明度
    vec3 color = vec3(0.0);
    float alpha = 1.0;

    // 设置步长
    float STEP_SIZE = 0.1;
    dir *= STEP_SIZE;

    // 计算初始值
    vec3 h = cross(pos, dir);
    float h2 = dot(h, h);

    // 迭代光线追踪
    for (int i = 0; i < 300; i++)
    {
        // 如果启用了黑洞渲染
        if (renderBlackHole > 0.5)
        {
            // 如果应用了引力透镜效应
            if (gravatationalLensing > 0.5)
            {
                // 计算加速度
                vec3 acc = accel(h2, pos);
                dir += acc;
            }

            // 到达事件视界
            if (dot(pos, pos) < 1.0)
            {
                return color; // 返回颜色
            }

            float minDistance = INFINITY;

            // 如果要绘制环
            if (adiskEnabled<0.5)
            {
                Ring ring;
                ring.center = vec3(0.0, 0.05, 0.0);
                ring.normal = vec3(0.0, 1.0, 0.0);
                ring.innerRadius = 2.0;
                ring.outerRadius = 6.0;
                ring.rotateSpeed = 0.08;
                ringColor(pos, dir, ring, minDistance, color);
            }
            else
            {
                adiskColor(pos, color, alpha);
            }
        }

        pos += dir; // 更新位置
    }

    // 采样天空盒颜色
    dir = rotateVectorUsingRodrigues(dir, vec3(0.0, 1.0, 0.0), time); // 旋转光线方向
    color += texture(galaxy, dir).rgb * alpha; // 添加天空盒颜色
    return color; // 返回最终颜色
}

mat3 lookAt(vec3 origin, vec3 target, float roll)
{
    vec3 right = vec3(sin(roll), cos(roll), 0.0);// 计算观察方向的右向量，即观察者位置到目标位置的方向
    vec3 front = normalize(target - origin);// 计算观察方向的正向向量
    vec3 up = normalize(cross(front, right));// 计算观察方向的上向量，通过叉乘得到
    vec3 v = normalize(cross(up, front));// 计算观察方向的侧向向量，通过叉乘得到
    return mat3(up, v, front);
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

        // 根据控制方式选择相机位置
    if (mouseControl > 0.5)
    {
        // 鼠标控制
        vec2 mouse = clamp(vec2(mouseX, mouseY) / resolution.xy, 0.0, 1.0) - 0.5;
        cameraPos = vec3(-cos(mouse.x * 10.0) * 15.0, mouse.y * 30.0,
                         sin(mouse.x * 10.0) * 15.0);
    }
    else if (frontView > 0.5)
    {
        // 前视图
        cameraPos = vec3(10.0, 1.0, 10.0);
    }
    else if (topView > 0.5)
    {
        // 顶视图
        cameraPos = vec3(15.0, 15.0, 0.0);
    }
    else
    {
        // 默认相机位置
        cameraPos = vec3(-cos(time * 0.1) * 15.0, sin(time * 0.1) * 15.0,
                         sin(time * 0.1) * 15.0);
    }

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
    if (raySphereIntersect(cameraPos, dir, sphere, t)&&renderBlackHole>0.5)
    {
        FragColor = vec4(0.0, 0.0, 0.0, 1.0); // 如果光线和球体相交，用黑色表示
    }
    else
    {
        // 如果光线和球体不相交，计算天空盒的颜色
        //dir = rotateVectorUsingRodrigues(dir, vec3(0.0, 1.0, 0.0), time);
        //vec3 skyboxColor = texture(skybox, dir).rgb;
        //FragColor = vec4(skyboxColor, 1.0);
        FragColor.xyz = traceColor(pos,dir);
    }
}