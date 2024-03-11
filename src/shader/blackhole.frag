#version 460 core

const float PI = 3.14159265359;
const float EPSILON = 0.0001;
const float INFINITY = 1000000.0;

out vec4 FragColor;

uniform vec2 resolution; // �ӿڷֱ��ʣ����أ�
uniform float time; // ʱ��

uniform float mouseX;
uniform float mouseY;
uniform float frontView = 0.0; // ǰ�ӽǱ�־
uniform float topView = 0.0; // ���ӽǱ�־
uniform float cameraRoll = 0.0; // �����ת�Ƕ�

uniform float gravatationalLensing = 1.0; // ����͸��ЧӦ��־
uniform float renderBlackHole = 1.0; // ��Ⱦ�ڶ���־
uniform float mouseControl = 1.0; // �����Ʊ�־
uniform float fovScale = 1.0; // �ӳ���������

uniform float adiskEnabled = 1.0; // �Ƿ���������Ч����1.0 Ϊ���ã�0.0 Ϊ����
uniform float adiskParticle = 1.0; // �Ƿ�ʹ�ÿ���Ч����1.0 Ϊʹ�ã�0.0 Ϊ��ʹ��
uniform float adiskHeight = 0.2; // ���̸߶�
uniform float adiskLit = 0.5; // ���̹���ǿ��
uniform float adiskDensityV = 1.0; // ��ֱ�����ϵ������ܶ�
uniform float adiskDensityH = 1.0; // ˮƽ�����ϵ������ܶ�
uniform float adiskNoiseScale = 1.0; // ������������
uniform float adiskNoiseLOD = 5.0; // �������� LOD��ϸ�ڼ���
uniform float adiskSpeed = 0.5; // ������ת�ٶ�

uniform samplerCube galaxy; // ��պ�����
uniform sampler2D colorMap; // ��պ�����

struct Sphere
{
    vec3 center; // ����
    float radius; // �뾶
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

// ʹ���û����������������û�
vec4 permute(vec4 x)
{
    return mod(((x * 34.0) + 1.0) * x, 289.0);
}

// ʹ��̩��չ����ƽ�����������������в���
vec4 taylorInvSqrt(vec4 r)
{
    return 1.79284291400159 - 0.85373472095314 * r;
}

float snoise(vec3 v)
{
  const vec2 C = vec2(1.0 / 6.0, 1.0 / 3.0); // �������ڼ����ݶ�
  const vec4 D = vec4(0.0, 0.5, 1.0, 2.0);   // �������������ĳ���

  // First corner
  vec3 i = floor(v + dot(v, C.yyy));
  vec3 x0 = v - i + dot(i, C.xxx);

  // Other corners
  vec3 g = step(x0.yzx, x0.xyz);
  vec3 l = 1.0 - g;
  vec3 i1 = min(g.xyz, l.zxy);
  vec3 i2 = max(g.xyz, l.zxy);

  // ����ƫ������
  //  x0 = x0 - 0. + 0.0 * C
  vec3 x1 = x0 - i1 + 1.0 * C.xxx;
  vec3 x2 = x0 - i2 + 2.0 * C.xxx;
  vec3 x3 = x0 - 1. + 3.0 * C.xxx;

  // Permutations�����У�
  i = mod(i, 289.0);
  vec4 p = permute(permute(permute(i.z + vec4(0.0, i1.z, i2.z, 1.0)) + i.y +
                           vec4(0.0, i1.y, i2.y, 1.0)) +
                   i.x + vec4(0.0, i1.x, i2.x, 1.0));

  // Gradients
  // �����ݶ�
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

// ���������뻷���ཻ�ľ���
float ringDistance(vec3 rayOrigin, vec3 rayDir, Ring ring)
{
  float denominator = dot(rayDir, ring.normal);
  float constant = -dot(ring.center, ring.normal);

  // ������߷����뷨��ƽ�У��򼸺�ƽ�У�
  if (abs(denominator) < EPSILON)
  {
    return -1.0;
  }
  else
  {
    // ���������뻷�Ľ������t
    float t = -(dot(rayOrigin, ring.normal) + constant) / denominator;
    if (t < 0.0)
    {
      return -1.0;
    }

    vec3 intersection = rayOrigin + t * rayDir;

    // ���㽻�㵽���ĵľ���
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

// ��Ԫ����ת
vec3 rotateVectorUsingQuaternion(vec3 position, vec3 axis, float angle)
{
    // ������ͽǶȴ�����Ԫ��
    float half_angle = (angle * 0.5) * PI / 180.0;
    vec4 qr = vec4(axis * sin(half_angle), cos(half_angle));

    // ������Ԫ���Ĺ���
    vec4 qr_conj = vec4(-qr.xyz, qr.w);

    // ��λ������ת��Ϊ��Ԫ����ʽ
    vec4 q_pos = vec4(position, 0.0);

    // ��Ԫ���˷�
    vec4 q_tmp = vec4(
        qr.w * q_pos.x + qr.y * q_pos.z - qr.z * q_pos.y,
        qr.w * q_pos.y + qr.z * q_pos.x - qr.x * q_pos.z,
        qr.w * q_pos.z + qr.x * q_pos.y - qr.y * q_pos.x,
        -qr.x * q_pos.x - qr.y * q_pos.y - qr.z * q_pos.z
    );

    // �ٴγ�����Ԫ���Ĺ���
    vec4 result = vec4(
        q_tmp.x * qr_conj.w + q_tmp.w * qr_conj.x + q_tmp.y * qr_conj.z - q_tmp.z * qr_conj.y,
        q_tmp.y * qr_conj.w + q_tmp.w * qr_conj.y + q_tmp.z * qr_conj.x - q_tmp.x * qr_conj.z,
        q_tmp.z * qr_conj.w + q_tmp.w * qr_conj.z + q_tmp.x * qr_conj.y - q_tmp.y * qr_conj.x,
        q_tmp.w * qr_conj.w - q_tmp.x * qr_conj.x - q_tmp.y * qr_conj.y - q_tmp.z * qr_conj.z
    );

    // ���ؽ������
    return result.xyz;
}
// �޵����˹��ʽ
vec3 rotateVectorUsingRodrigues(vec3 position, vec3 axis, float angle)
{
    // ���Ƕ�ת��Ϊ����
    float radians = angle * PI / 180.0;

    // ������ת�������
    vec3 rotatedVector = position * cos(radians) +
                         cross(axis, position) * sin(radians) +
                         axis * dot(axis, position) * (1.0 - cos(radians));

    return rotatedVector;
}

void ringColor(vec3 rayOrigin, vec3 rayDir, Ring ring, inout float minDistance,
               inout vec3 color)
{
  // ���������뻷�Ľ������
  float distance = ringDistance(rayOrigin, normalize(rayDir), ring);
  // �����������ں���Χ��
  if (distance >= EPSILON && distance < minDistance && distance <= length(rayDir) + EPSILON)
  {
    // ������С����
    minDistance = distance;

    // ���㽻������
    vec3 intersection = rayOrigin + normalize(rayDir) * minDistance;
    vec3 ringColor;

    // ���㻷����ɫ
    {
      // ���㽻�㵽ԭ��ľ���
      float dist = length(intersection);

      // ���㻷�ϵĴ�ֱ����v
      float v = clamp((dist - ring.innerRadius) /
                          (ring.outerRadius - ring.innerRadius),
                      0.0, 1.0);

      // ���㻷�ϵ�ˮƽ����u
      vec3 base = cross(ring.normal, vec3(0.0, 0.0, 1.0));
      float angle = acos(dot(normalize(base), normalize(intersection)));
      if (dot(cross(base, intersection), ring.normal) < 0.0)
        angle = -angle;

      float u = 0.5 - 0.5 * angle / PI;
      // HACK
      // ˮƽ����u��ʱ��仯
      u += time * ring.rotateSpeed;

      // ���û��Ļ�����ɫ
      vec3 color = vec3(0.0, 0.5, 1.0);
      // HACK
      // ����͸����
      float alpha = 0.5;
      ringColor = vec3(color);
    }

    // ��������ɫ�ӵ�������ɫ��
    color += ringColor;
  }
}

// ���ݷ���������ȡȫ��ͼ�е���ɫ
vec3 panoramaColor(sampler2D tex, vec3 dir)
{
  // ����������ϵ�µ���������
    vec2 uv = vec2(0.5 - atan(dir.z, dir.x) / PI * 0.5, 0.5 - asin(dir.y) / PI);
    // ʹ��������ϵ�µ����������ȡ������ɫ������
    return texture2D(tex, uv).rgb;
}

float sqrLength(vec3 a)
{
    return dot(a, a);
}

vec3 toSpherical(vec3 p)
{

  float rho = sqrt((p.x * p.x) + (p.y * p.y) + (p.z * p.z));// �����������еľ������ rho
  float theta = atan(p.z, p.x);// �����������еķ�λ�� theta
  float phi = asin(p.y / rho);// �����������еļ��� phi

  return vec3(rho, theta, phi);// ����������
}

void adiskColor(vec3 pos, inout vec3 color, inout float alpha)
{
    float innerRadius = 2.6;
    float outerRadius = 12.0;

    // ���ݵ��ڶ����ĵľ������Լ�С�ܶ�
    float density = max(0.0, 1.0 - length(pos.xyz / vec3(outerRadius, adiskHeight, outerRadius)));
    if (density < 0.001)
    {
        return;
    }

    density *= pow(1.0 - abs(pos.y) / adiskHeight, adiskDensityV);

    // ���뾶С�����ȶ�Բ���ʱ���������ܶ�����Ϊ0
    density *= smoothstep(innerRadius, innerRadius * 1.1, length(pos));

    // ���ܶȺ�Сʱ���˳�����
    if (density < 0.001)
    {
        return;
    }

    vec3 sphericalCoord = toSpherical(pos);

    // ���� rho �� phi���Ա��������Ӿ�����ʾΪ��ȷ�ı���
    sphericalCoord.y *= 2.0;
    sphericalCoord.z *= 4.0;

    // ���ݾ�����������ܶ�
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

    // ��ȡ������ɫ
    vec3 dustColor = texture(colorMap, vec2(sphericalCoord.x / outerRadius, 0.5)).rgb;

    // ����������ɫ
    color += density * adiskLit * dustColor * alpha * abs(noise);
}

vec3 traceColor(vec3 pos, vec3 dir)
{
    // ��ʼ����ɫ��͸����
    vec3 color = vec3(0.0);
    float alpha = 1.0;

    // ���ò���
    float STEP_SIZE = 0.1;
    dir *= STEP_SIZE;

    // �����ʼֵ
    vec3 h = cross(pos, dir);
    float h2 = dot(h, h);

    // ��������׷��
    for (int i = 0; i < 300; i++)
    {
        // ��������˺ڶ���Ⱦ
        if (renderBlackHole > 0.5)
        {
            // ���Ӧ��������͸��ЧӦ
            if (gravatationalLensing > 0.5)
            {
                // ������ٶ�
                vec3 acc = accel(h2, pos);
                dir += acc;
            }

            // �����¼��ӽ�
            if (dot(pos, pos) < 1.0)
            {
                return color; // ������ɫ
            }

            float minDistance = INFINITY;

            // ���Ҫ���ƻ�
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

        pos += dir; // ����λ��
    }

    // ������պ���ɫ
    dir = rotateVectorUsingRodrigues(dir, vec3(0.0, 1.0, 0.0), time); // ��ת���߷���
    color += texture(galaxy, dir).rgb * alpha; // �����պ���ɫ
    return color; // ����������ɫ
}

mat3 lookAt(vec3 origin, vec3 target, float roll)
{
    vec3 right = vec3(sin(roll), cos(roll), 0.0);// ����۲췽��������������۲���λ�õ�Ŀ��λ�õķ���
    vec3 front = normalize(target - origin);// ����۲췽�����������
    vec3 up = normalize(cross(front, right));// ����۲췽�����������ͨ����˵õ�
    vec3 v = normalize(cross(up, front));// ����۲췽��Ĳ���������ͨ����˵õ�
    return mat3(up, v, front);
}

// �жϹ��ߺ������Ƿ��ཻ���������ཻ��Ĳ��� t
bool raySphereIntersect(vec3 rayOrigin, vec3 rayDir, Sphere sphere, out float t)
{
    vec3 oc = rayOrigin - sphere.center;
    float a = dot(rayDir, rayDir);
    float b = 2.0 * dot(oc, rayDir);
    float c = dot(oc, oc) - sphere.radius * sphere.radius;
    float discriminant = b * b - 4.0 * a * c;

    if (discriminant < 0.0)
    {
        return false; // û���ཻ
    }
    else
    {
        float t0 = (-b - sqrt(discriminant)) / (2.0 * a);
        float t1 = (-b + sqrt(discriminant)) / (2.0 * a);
        t = min(t0, t1);
        return true; // �����ཻ��
    }
}

void main()
{
    mat3 view; // ��ͼ����
    vec3 cameraPos; // ���λ��

        // ���ݿ��Ʒ�ʽѡ�����λ��
    if (mouseControl > 0.5)
    {
        // ������
        vec2 mouse = clamp(vec2(mouseX, mouseY) / resolution.xy, 0.0, 1.0) - 0.5;
        cameraPos = vec3(-cos(mouse.x * 10.0) * 15.0, mouse.y * 30.0,
                         sin(mouse.x * 10.0) * 15.0);
    }
    else if (frontView > 0.5)
    {
        // ǰ��ͼ
        cameraPos = vec3(10.0, 1.0, 10.0);
    }
    else if (topView > 0.5)
    {
        // ����ͼ
        cameraPos = vec3(15.0, 15.0, 0.0);
    }
    else
    {
        // Ĭ�����λ��
        cameraPos = vec3(-cos(time * 0.1) * 15.0, sin(time * 0.1) * 15.0,
                         sin(time * 0.1) * 15.0);
    }

    vec3 target = vec3(0.0, 0.0, 0.0); // Ŀ��λ��Ϊԭ��
    view = lookAt(cameraPos, target, radians(cameraRoll)); // ������ͼ����

    vec2 uv = gl_FragCoord.xy / resolution.xy - vec2(0.5); // �������ص�����
    uv.x *= resolution.x / resolution.y; // ������Ļ��߱ȵ��� x ����

    vec3 dir = normalize(vec3(-uv.x * fovScale, uv.y * fovScale, 1.0)); // �������߷���
    vec3 pos = cameraPos; // ���ó�ʼλ��Ϊ���λ��
    dir = view * dir; // Ӧ����ͼ�������߷���

    // ��������
    Sphere sphere;
    sphere.center = vec3(0.0, 0.0, 0.0); // ����λ�� (0.0, 0.0, -5.0)
    sphere.radius = 1.0; // �뾶Ϊ 1.0

    // �жϹ��ߺ������Ƿ��ཻ
    float t;
    if (raySphereIntersect(cameraPos, dir, sphere, t)&&renderBlackHole>0.5)
    {
        FragColor = vec4(0.0, 0.0, 0.0, 1.0); // ������ߺ������ཻ���ú�ɫ��ʾ
    }
    else
    {
        // ������ߺ����岻�ཻ��������պе���ɫ
        //dir = rotateVectorUsingRodrigues(dir, vec3(0.0, 1.0, 0.0), time);
        //vec3 skyboxColor = texture(skybox, dir).rgb;
        //FragColor = vec4(skyboxColor, 1.0);
        FragColor.xyz = traceColor(pos,dir);
    }
}