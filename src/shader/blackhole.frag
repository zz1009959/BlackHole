#version 460 core

const float PI = 3.14159265359;
const float EPSILON = 0.0001;
const float INFINITY = 1000000.0;

out vec4 FragColor;

uniform vec2 resolution; // �ӿڷֱ��ʣ����أ�
//uniform vec3 cameraPos; // ���λ��
uniform float time; // ʱ��
//uniform mat4 view; // �������ͼ����

uniform float mouseX;
uniform float mouseY;
uniform float frontView = 0.0; // ǰ�ӽǱ�־
uniform float topView = 0.0; // ���ӽǱ�־
uniform float cameraRoll = 0.0; // �����ת�Ƕ�

uniform float gravatationalLensing = 1.0; // ����͸��ЧӦ��־
uniform float renderBlackHole = 1.0; // ��Ⱦ�ڶ���־
uniform float mouseControl = 1.0; // �����Ʊ�־
uniform float fovScale = 1.0; // �ӳ���������

struct Sphere
{
    vec3 center; // ����
    float radius; // �뾶
};

uniform samplerCube skybox; // ��պ�����

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

mat3 lookAt(vec3 origin, vec3 target, float roll)
{
    vec3 rr = vec3(sin(roll), cos(roll), 0.0);// ����۲췽��������������۲���λ�õ�Ŀ��λ�õķ���
    vec3 ww = normalize(target - origin);// ����۲췽�����������
    vec3 uu = normalize(cross(ww, rr));// ����۲췽�����������ͨ����˵õ�
    vec3 vv = normalize(cross(uu, ww));// ����۲췽��Ĳ���������ͨ����˵õ�
    return mat3(uu, vv, ww);
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

    vec2 mouse = clamp(vec2(mouseX, mouseY) / resolution.xy, 0.0, 1.0) - 0.5;
    cameraPos = vec3(-cos(mouse.x * 10.0) * 15.0, mouse.y * 30.0,sin(mouse.x * 10.0) * 15.0);

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
    if (raySphereIntersect(cameraPos, dir, sphere, t))
    {
        FragColor = vec4(1.0, 1.0, 1.0, 1.0); // ������ߺ������ཻ���ú�ɫ��ʾ
    }
    else
    {
        // ������ߺ����岻�ཻ��������պе���ɫ
        dir = rotateVectorUsingQuaternion(dir, vec3(0.0, 1.0, 0.0), time);
        vec3 skyboxColor = texture(skybox, dir).rgb;
        FragColor = vec4(skyboxColor, 1.0);
    }
}