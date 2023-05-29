#include "stdio.h"

#include <vector>

#include <d3d11.h>
#include <d3dcommon.h>
#include <d3dcompiler.h>
#include <dxgi.h>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

const char* WINDOW_CLASS = "rtow_window_class";

constexpr int WINDOW_DEFAULT_WIDTH = 1024;
constexpr int WINDOW_DEFAULT_HEIGHT = WINDOW_DEFAULT_WIDTH / (16.0 / 9.0);

struct Window
{
	int width, height;
	const char* title;
	HWND handle;
	HDC hdc;
};

struct Sphere
{
	float origin[3];
	float radius;
	int mat_index;

	Sphere(float x, float y, float z, float r, int mat)
	{
		origin[0] = x;
		origin[1] = y;
		origin[2] = z;
		radius = r;
		mat_index = mat;
	}
};

struct Material
{
	enum KIND
	{
		KIND_LAMBERTIAN,
		KIND_METAL,
		KIND_DIELECTRIC,
	};

	KIND kind;
	float albedo[3];
	float fuzz;
	float ir;
};

struct World
{
	std::vector<Sphere> spheres;
	std::vector<Material> materials;
};

int world_push(World& self, const Material& m)
{
	self.materials.push_back(m);
	return self.materials.size() - 1;
}

Material lambertian(float r, float g, float b)
{
	Material self{};
	self.kind = Material::KIND_LAMBERTIAN;
	self.albedo[0] = r;
	self.albedo[1] = g;
	self.albedo[2] = b;
	return self;
}

Material metal(float r, float g, float b, float f)
{
	Material self{};
	self.kind = Material::KIND_METAL;
	self.albedo[0] = r;
	self.albedo[1] = r;
	self.albedo[2] = r;
	self.fuzz = f;
	return self;
}

Material dielectric(float ir)
{
	Material self{};
	self.kind = Material::KIND_DIELECTRIC;
	self.ir = ir;
	return self;
}


struct Renderer
{
	bool ready;
	IDXGIFactory* factory;
	IDXGIAdapter* adapter;
	ID3D11Device* device;
	ID3D11DeviceContext* context;
	IDXGISwapChain* swapchain;
	ID3D11RenderTargetView* render_target_view;
	ID3D11Buffer* screen_rect_vertices;
	ID3D10Blob* compiled_vs_shader;
	ID3D11VertexShader* screen_rect_vertex_shader;
	ID3D11PixelShader* screen_rect_pixel_shader;
	ID3D11RasterizerState* screen_rect_rasterizer_state;
	ID3D11InputLayout* screen_rect_input_layout;
	ID3D11Texture2D* texture;
	ID3D11ShaderResourceView* texture_resource_view;
	ID3D11UnorderedAccessView* texture_unordered_view;
	ID3D11SamplerState* texture_sampler;
	ID3D11ComputeShader* raytrace_compute_shader;
	ID3D11Buffer* raytrace_spheres_buffer;
	ID3D11ShaderResourceView* raytrace_spheres_buffer_resource_view;
	ID3D11Buffer* raytrace_materials_buffer;
	ID3D11ShaderResourceView* raytrace_materials_buffer_resource_view;
};

void renderer_draw(Renderer& self)
{
	if (self.ready == false)
		return;

	self.context->CSSetShader(self.raytrace_compute_shader, NULL, 0);
	self.context->CSSetUnorderedAccessViews(0, 1, &self.texture_unordered_view, nullptr);
	self.context->CSSetShaderResources(1, 1, &self.raytrace_spheres_buffer_resource_view);
	self.context->CSSetShaderResources(2, 1, &self.raytrace_materials_buffer_resource_view);
	D3D11_TEXTURE2D_DESC texture_desc{};
	self.texture->GetDesc(&texture_desc);
	// 1 + ((total_size.x - 1) / tile_size.x)
	UINT x = ((texture_desc.Width - 1) / 16) + 1;
	UINT y = ((texture_desc.Height - 1) / 16) + 1;
	self.context->Dispatch(x, y, 1);

	ID3D11UnorderedAccessView* unbind_uavs[] = {nullptr};
	self.context->CSSetUnorderedAccessViews(0, 1, unbind_uavs, nullptr);

	ID3D11ShaderResourceView* unbind_srvs[] = {nullptr};
	self.context->CSSetShaderResources(1, 1, unbind_srvs);
	self.context->CSSetShaderResources(2, 1, unbind_srvs);

	self.context->OMSetRenderTargets(1, &self.render_target_view, nullptr);
	D3D11_VIEWPORT viewport{};
	viewport.Width = WINDOW_DEFAULT_WIDTH;
	viewport.Height = WINDOW_DEFAULT_HEIGHT;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	self.context->RSSetViewports(1, &viewport);

	self.context->RSSetState(self.screen_rect_rasterizer_state);
	self.context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	self.context->VSSetShader(self.screen_rect_vertex_shader, NULL, 0);
	self.context->PSSetShader(self.screen_rect_pixel_shader, NULL, 0);
	self.context->IASetInputLayout(self.screen_rect_input_layout);
	self.context->PSSetShaderResources(0, 1, &self.texture_resource_view);
	self.context->PSSetSamplers(0, 1, &self.texture_sampler);

	UINT offset = 0;
	UINT stride = 2 * sizeof(float);
	self.context->IASetVertexBuffers(0, 1, &self.screen_rect_vertices, &stride, &offset);

	self.context->Draw(6, 0);

	self.context->PSSetShaderResources(0, 1, unbind_srvs);

	self.swapchain->Present(0, 0);
}

inline uint32_t xor_shift_32_rand()
{
	static uint32_t state = 42;
	uint32_t x = state;
	x ^= x << 13;
	x ^= x >> 17;
	x ^= x << 15;
	state = x;
	return x;
}

float random_double()
{
	return xor_shift_32_rand() / float(UINT32_MAX);
}

float random_double(float min, float max)
{
	return min + (max - min) * random_double();
}

World random_scene()
{
	World world{};

	auto ground_material = world_push(world, lambertian(0.5, 0.5, 0.5));
	world.spheres.push_back(Sphere{0, -1000, 0, 1000, ground_material});

	for (int a = -11; a < 11; ++a)
	{
		for (int b = -11; b < 11; ++b)
		{
			auto choose_mat = random_double();

			float x = a + 0.9f * random_double();
			float y = 0.2f;
			float z = b + 0.9f * random_double();

			auto dist_x = x - 4;
			auto dist_y = y - 0.2;
			auto dist_z = z - 0;

			float dist = sqrt(dist_x * dist_x + dist_y * dist_y + dist_z * dist_z);

			if (dist > 0.9)
			{
				int sphere_material;

				if (choose_mat < 0.8)
				{
					sphere_material = world_push(world, lambertian(random_double() * random_double(), random_double() * random_double(), random_double() * random_double()));
					world.spheres.push_back(Sphere{x, y, z, 0.2, sphere_material});
				}
				else if (choose_mat < 0.95)
				{
					sphere_material = world_push(world, metal(random_double(0.5, 1), random_double(0.5, 1), random_double(0.5, 1), random_double(0, 0.5)));
					world.spheres.push_back(Sphere{x, y, z, 0.2, sphere_material});
				}
				else
				{
					sphere_material = world_push(world, dielectric(1.5));
					world.spheres.push_back(Sphere{x, y, z, 0.2, sphere_material});
				}
			}
		}
	}

	auto material1 = world_push(world, dielectric(1.5));
	world.spheres.push_back(Sphere{0, 1, 0, 1, material1});

	auto material2 = world_push(world, lambertian(0.4, 0.2, 0.1));
	world.spheres.push_back(Sphere{-4, 1, 0, 1, material2});

	auto material3 = world_push(world, lambertian(0.7, 0.6, 0.5));
	world.spheres.push_back(Sphere{4, 1, 0, 1, material3});

	return world;
}

void renderer_setup_resources(Renderer& self, Window& window)
{
	static float RECT_VERTICES[] = {
		-1.0, -1.0,
		 1.0,  1.0,
		-1.0,  1.0,

		-1.0, -1.0,
		 1.0, -1.0,
		 1.0,  1.0
	};

	static const char* RECT_VERTEX_SHADER = R"SHADER(
		struct VS_Input
		{
			float2 pos: POSITION;
		};

		struct VS_Output
		{
			float4 pos: SV_POSITION;
			float2 uv: TEXCOORD0;
		};

		VS_Output main(VS_Input input)
		{
			VS_Output output;
			output.pos = float4(input.pos, 0, 1);
			output.uv = (input.pos + float2(1, 1)) / float2(2, 2);
			return output;
		}
	)SHADER";

	static const char* RECT_PIXEL_SHADER = R"SHADER(
		struct PS_Input
		{
			float4 pos: SV_POSITION;
			float2 uv: TEXCOORD0;
		};

		Texture2D tex: register(t0);
		SamplerState tex_sampler: register(s0);

		float4 main(PS_Input input): SV_TARGET
		{
			return tex.Sample(tex_sampler, input.uv);
			// return float4(input.uv, 0.25, 1);
		}
	)SHADER";

	static const char* RAYTRACE_COMPUTE_SHADER = R"SHADER(
struct Ray
{
	float3 origin;
	float3 dir;
};

struct Sphere
{
	float3 center;
	float radius;
	int mat_index;
};

struct Hit_Record
{
	float3 p;
	float3 normal;
	int mat_index;
	float t;
	bool front_face;
};

struct Camera
{
	float3 origin;
	float3 lower_left_corner;
	float3 horizontal;
	float3 vertical;
	float3 u, v, w;
	float lens_radius;
};

struct Random_Series
{
	uint state;
};

#define Material_Kind_Lambertian 0
#define Material_Kind_Metal 1
#define Material_Kind_Dielectric 2

struct Material
{
	int kind;
	float3 albedo;
	float fuzz;
	float ir;
};

RWTexture2D<float4> output: register(u0);

StructuredBuffer<Sphere> spheres: register(t1);
StructuredBuffer<Material> materials: register(t2);

static const float pi = 3.1415926535897932385;

inline uint xor_shift_32_rand(inout uint state)
{
	uint x = state;
	x ^= x << 13;
	x ^= x >> 17;
	x ^= x << 15;
	state = x;
	return x;
}

float random_double(inout uint series)
{
	return (xor_shift_32_rand(series) & 0xFFFFFF) / 16777216.0f;
}

float random_double_in_range(inout uint series, float min, float max)
{
	return min + (max - min) * random_double(series);
}

float3 random_in_unit_disk(inout uint series)
{
	float a = random_double(series) * 2.0 * pi;
	float2 xy = float2(cos(a), sin(a));
	xy *= sqrt(random_double(series));
	return float3(xy, 0);
}

float3 random_unit_vector(inout uint series)
{
	float z = random_double_in_range(series, -1, 1);
	float a = random_double_in_range(series, 0, 2 * pi);
	float r = sqrt(1.0 - z * z);
	float x = r * cos(a);
	float y = r * sin(a);
	return float3(x, y, z);
}

float3 random_in_unit_sphere(inout uint series)
{
	float z = random_double_in_range(series, -1, 1);
	float t = random_double_in_range(series, 0, 2 * pi);
	float r = sqrt(max(0, 1 - z * z));
	float x = r * cos(t);
	float y = r * sin(t);
	float3 res = float3(x, y, z);
	res *= pow(random_double(series), 1.0 / 3.0);
	return res;
}

float degrees_to_radians(float degrees)
{
	return degrees * pi / 180.0;
}

float3 unit_vector(float3 v)
{
	return normalize(v);
}

Camera camera_new(float3 lookfrom, float3 lookat, float3 vup, float vertical_fov_degrees, float aspect_ratio, float aperture, float focus_dist)
{
	float theta = degrees_to_radians(vertical_fov_degrees);
	float h = tan(theta / 2);
	float viewport_height = 2.0 * h;
	float viewport_width = aspect_ratio * viewport_height;

	Camera cam;
	cam.w = unit_vector(lookfrom - lookat);
	cam.u = unit_vector(cross(vup, cam.w));
	cam.v = cross(cam.w, cam.u);

	cam.origin = lookfrom;
	cam.horizontal = focus_dist * viewport_width * cam.u;
	cam.vertical = focus_dist * viewport_height * cam.v;
	cam.lower_left_corner = cam.origin - cam.horizontal / 2 - cam.vertical / 2 - focus_dist * cam.w;
	cam.lens_radius = aperture / 2;
	return cam;
}

Ray camera_get_ray(Camera cam, inout uint series, float s, float t)
{
	float3 rd = cam.lens_radius * random_in_unit_disk(series);
	float offset = cam.u * rd.x + cam.v * rd.y;

	Ray ray;
	ray.origin = cam.origin + offset;
	ray.dir = cam.lower_left_corner + s * cam.horizontal + t * cam.vertical - cam.origin - offset;
	return ray;
}

float2 texture_size(RWTexture2D<float4> tex)
{
	uint width, height;
	tex.GetDimensions(width, height);
	return float2(width, height);
}

float length_squared(float3 v)
{
	return dot(v, v);
}

float3 ray_at(Ray r, float t)
{
	return r.origin + t * r.dir;
}

bool sphere_hit(Sphere sphere, Ray r, float t_min, float t_max, out Hit_Record rec)
{
	float a = length_squared(r.dir);
	float3 oc = r.origin - sphere.center;
	float half_b = dot(r.dir, oc);
	float c = length_squared(oc) - sphere.radius * sphere.radius;

	float discriminant = half_b * half_b - a * c;
	if (discriminant < 0) return false;
	float sqrtd = sqrt(discriminant);

	// find the nearest of 2 possible solutions
	float root = (-half_b - sqrtd) / a;
	if (root < t_min || root > t_max)
	{
		root = (-half_b + sqrtd) / a;
		if (root < t_min || root > t_max)
			return false;
	}

	rec.t = root;
	rec.p = ray_at(r, rec.t);
	float3 outward_normal = (rec.p - sphere.center) / sphere.radius;
	rec.front_face = dot(r.dir, outward_normal) < 0;
	rec.normal = rec.front_face ? outward_normal : -outward_normal;
	rec.mat_index = sphere.mat_index;
	return true;
}

bool world_hit(Ray r, float t_min, float t_max, inout Hit_Record rec, uint spheres_count)
{
	Hit_Record temp_rec;
	bool hit_anything = false;
	float closest_so_far = t_max;

	for (uint i = 0; i < spheres_count; ++i)
	{
		if (sphere_hit(spheres[i], r, t_min, closest_so_far, temp_rec))
		{
			hit_anything = true;
			closest_so_far = temp_rec.t;
			rec = temp_rec;
		}
	}

	return hit_anything;
}

float reflectance(float cosine, float ref_idx)
{
	float r0 = (1 - ref_idx) / (1 + ref_idx);
	r0 = r0 * r0;
	return r0 + (1 - r0) * pow((1 - cosine), 5);
}

bool material_scatter(Material mat, Ray r, Hit_Record rec, out float3 attenuation, out Ray scattered, inout uint series)
{
	switch (mat.kind)
	{
	case Material_Kind_Lambertian:
	{
		float3 scatter_direction = rec.normal + random_unit_vector(series);

		if (all(abs(scatter_direction) < float3(1e-8, 1e-8, 1e-8)))
			scatter_direction = rec.normal;

		scattered.origin = rec.p;
		scattered.dir = scatter_direction;
		attenuation = mat.albedo;
		return true;
	}
	case Material_Kind_Metal:
	{
		float3 reflected = reflect(unit_vector(r.dir), rec.normal);
		scattered.origin = rec.p;
		scattered.dir = reflected + mat.fuzz * random_in_unit_sphere(series);
		attenuation = mat.albedo;
		return dot(scattered.dir, rec.normal) > 0;
	}
	case Material_Kind_Dielectric:
	{
		attenuation = float3(1, 1, 1);
		float refraction_ratio = rec.front_face ? 1.0/mat.ir : mat.ir;

		float3 unit_direction = unit_vector(r.dir);
		float cos_theta = min(dot(-unit_direction, rec.normal), 1.0);
		float sin_theta = sqrt(1.0 - cos_theta*cos_theta);

		bool cannot_refract = refraction_ratio * sin_theta > 1.0;
		float3 direction = float3(0, 0, 0);

		if (cannot_refract || reflectance(cos_theta, refraction_ratio) > random_double(series))
			direction = reflect(unit_direction, rec.normal);
		else
			direction = refract(unit_direction, rec.normal, refraction_ratio);

		scattered.origin = rec.p;
		scattered.dir = direction;
		return true;
	}
	default:
		scattered.origin = float3(0, 0, 0);
		scattered.dir = float3(0, 0, 0);
		attenuation = float3(0, 0, 0);
		return false;
	}
}

float3 ray_color(Ray r, uint spheres_count, inout uint series)
{
	Hit_Record rec;

	if (world_hit(r, 0.001, 1.#INF, rec, spheres_count))
	{
		Ray scattered;
		float3 attenuation;
		Material mat = materials[rec.mat_index];
		if (material_scatter(mat, r, rec, attenuation, scattered, series))
		{
			return attenuation;
		}
		return float3(0, 0, 0);
	}

	float3 unit_direction = normalize(r.dir);
	float t = 0.5 * (unit_direction.y + 1.0);
	return lerp(float3(1, 1, 1), float3(0.5, 0.7, 1.0), t);
}

[numthreads(16, 16, 1)]
void main(uint3 DTid: SV_DispatchThreadID)
{
	float aspect_ratio = 16.0 / 9.0;
	float2 size = texture_size(output);
	float image_width = size.x;
	float image_height = size.y;

	uint spheres_count = 0;
	uint sphere_size = 0;
	spheres.GetDimensions(spheres_count, sphere_size);

	uint random_series = DTid.y * image_height + DTid.x;

	float3 lookfrom = float3(13, 2, 3);
	float3 lookat = float3(0, 0, 0);
	float3 vup = float3(0, 1, 0);
	float dist_to_focus = 10;
	float aperture = 0.1;
	Camera cam = camera_new(lookfrom, lookat, vup, 20, aspect_ratio, aperture, dist_to_focus);
	// Camera cam = camera_new(float3(0, 2, 3), float3(0, 0, 0), float3(0, 1, 0), 60, aspect_ratio, aperture, 3);

	float samples_per_pixel = 10;
	float3 color = float3(0, 0, 0);
	for (int i = 0; i < samples_per_pixel; ++i)
	{
		float u = (DTid.x + random_double(random_series)) / (image_width - 1);
		float v = (DTid.y + random_double(random_series)) / (image_height - 1);
		Ray ray = camera_get_ray(cam, random_series, u, v);
		color += ray_color(ray, spheres_count, random_series);
	}
	float scale = 1.0 / samples_per_pixel;
	output[DTid.xy] = float4(sqrt(color * scale), 1);

	// float2 normalized_index = float2(DTid.xy) / size;
	// output[DTid.xy] = float4(normalized_index.x, normalized_index.y, 0.25, 1);
}
	)SHADER";

	// setup screen rect vertex buffer;
	{
		D3D11_BUFFER_DESC desc{};
		desc.ByteWidth = sizeof(RECT_VERTICES);
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER,
		desc.Usage = D3D11_USAGE_IMMUTABLE;
		D3D11_SUBRESOURCE_DATA data_desc{};
		data_desc.pSysMem = RECT_VERTICES;

		auto res = self.device->CreateBuffer(&desc, &data_desc, &self.screen_rect_vertices);
		if (FAILED(res))
		{
			fprintf(stderr, "failed to create buffer");
			exit(EXIT_FAILURE);
		}
	}

	// vertex shader compilation
	{
		ID3D10Blob* error = nullptr;
		UINT compile_flags = 0;
		#if defined(_DEBUG)
			compile_flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
		#endif

		auto res = D3DCompile(RECT_VERTEX_SHADER, strlen(RECT_VERTEX_SHADER), NULL, NULL, NULL, "main", "vs_5_0", compile_flags, 0, &self.compiled_vs_shader, &error);
		if (FAILED(res))
		{
			fprintf(stderr, "failed to compile shader, %s\n", (char*)error->GetBufferPointer());
			exit(EXIT_FAILURE);
		}

		res = self.device->CreateVertexShader(
			self.compiled_vs_shader->GetBufferPointer(),
			self.compiled_vs_shader->GetBufferSize(),
			NULL,
			&self.screen_rect_vertex_shader
		);
		if (FAILED(res))
		{
			fprintf(stderr, "failed to create vertex shader\n");
			exit(EXIT_FAILURE);
		}
	}

	// pixel shader compilation
	{
		ID3D10Blob* error = nullptr;
		UINT compile_flags = 0;
		#if defined(_DEBUG)
			compile_flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
		#endif

		ID3D10Blob* shader_blob;
		auto res = D3DCompile(RECT_PIXEL_SHADER, strlen(RECT_PIXEL_SHADER), NULL, NULL, NULL, "main", "ps_5_0", compile_flags, 0, &shader_blob, &error);
		if (FAILED(res))
		{
			fprintf(stderr, "failed to compile shader, %s\n", (char*)error->GetBufferPointer());
			exit(EXIT_FAILURE);
		}

		res = self.device->CreatePixelShader(
			shader_blob->GetBufferPointer(),
			shader_blob->GetBufferSize(),
			NULL,
			&self.screen_rect_pixel_shader
		);
		if (FAILED(res))
		{
			fprintf(stderr, "failed to create pixel shader\n");
			exit(EXIT_FAILURE);
		}

		shader_blob->Release();
	}

	// create input layout
	{
		D3D11_INPUT_ELEMENT_DESC input_layout_desc[1];
		::memset(input_layout_desc, 0, sizeof(input_layout_desc));
		for (size_t i = 0; i < ARRAYSIZE(input_layout_desc); ++i)
		{
			auto& attribute = input_layout_desc[i];
			attribute.SemanticName = "POSITION";
			attribute.SemanticIndex = 0;
			attribute.Format = DXGI_FORMAT_R32G32_FLOAT;
			attribute.InputSlot = i;
		}
		auto res = self.device->CreateInputLayout(input_layout_desc, ARRAYSIZE(input_layout_desc), self.compiled_vs_shader->GetBufferPointer(), self.compiled_vs_shader->GetBufferSize(), &self.screen_rect_input_layout);
		if (FAILED(res))
		{
			fprintf(stderr, "failed to create input layout");
			exit(EXIT_FAILURE);
		}
	}

	// create rasterizer state
	{
		D3D11_RASTERIZER_DESC raster_desc{};
		raster_desc.CullMode = D3D11_CULL_NONE;
		raster_desc.FillMode = D3D11_FILL_SOLID;
		raster_desc.FrontCounterClockwise = true;
		auto res = self.device->CreateRasterizerState(&raster_desc, &self.screen_rect_rasterizer_state);
		if (FAILED(res))
		{
			fprintf(stderr, "failed to create rasterizer state");
			exit(EXIT_FAILURE);
		}
	}

	// create swapchain
	{
		DXGI_SWAP_CHAIN_DESC desc{};
		ZeroMemory(&desc, sizeof(desc));
		desc.BufferCount = 1;
		desc.BufferDesc.Width = window.width;
		desc.BufferDesc.Height = window.height;
		desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

		IDXGIOutput* output = nullptr;
		auto res = self.adapter->EnumOutputs(0, &output);
		if (FAILED(res))
		{
			fprintf(stderr, "failed to enum output of DXGIAdapter");
			exit(EXIT_FAILURE);
		}

		UINT modes_count = 0;
		res = output->GetDisplayModeList(desc.BufferDesc.Format, DXGI_ENUM_MODES_INTERLACED, &modes_count, NULL);
		if (FAILED(res))
		{
			fprintf(stderr, "failed to enum available modes for this DXGIOutput");
			exit(EXIT_FAILURE);
		}

		std::vector<DXGI_MODE_DESC> modes{modes_count};
		res = output->GetDisplayModeList(desc.BufferDesc.Format, DXGI_ENUM_MODES_INTERLACED, &modes_count, modes.data());
		if (FAILED(res))
		{
			fprintf(stderr, "failed to enum available modes for this DXGIOutput");
			exit(EXIT_FAILURE);
		}
		output->Release();

		for (const auto& mode: modes)
		{
			if (mode.Width == desc.BufferDesc.Width && mode.Height == desc.BufferDesc.Height)
			{
				desc.BufferDesc.RefreshRate = mode.RefreshRate;
			}
		}

		desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		desc.OutputWindow = window.handle;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Windowed = TRUE;
		desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

		res = self.factory->CreateSwapChain(self.device, &desc, &self.swapchain);
		if (FAILED(res))
		{
			fprintf(stderr, "failed to create swapchain");
			exit(EXIT_FAILURE);
		}

		ID3D11Texture2D* color_buffer = nullptr;
		res = self.swapchain->GetBuffer(0, IID_PPV_ARGS(&color_buffer));
		if (FAILED(res))
		{
			fprintf(stderr, "failed to get swapchain buffer");
			exit(EXIT_FAILURE);
		}

		res = self.device->CreateRenderTargetView(color_buffer, nullptr, &self.render_target_view);
		if (FAILED(res))
		{
			fprintf(stderr, "failed to get render target view");
			exit(EXIT_FAILURE);
		}
	}

	// create texture
	{
		D3D11_TEXTURE2D_DESC texture_desc{};
		texture_desc.ArraySize = 1;
		texture_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
		texture_desc.Width = WINDOW_DEFAULT_WIDTH;
		texture_desc.Height = WINDOW_DEFAULT_HEIGHT;
		texture_desc.Usage = D3D11_USAGE_DEFAULT;
		texture_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		texture_desc.SampleDesc.Count = 1;
		texture_desc.MipLevels = 1;
		auto res = self.device->CreateTexture2D(&texture_desc, nullptr, &self.texture);
		if (FAILED(res))
		{
			fprintf(stderr, "failed to create texture2d");
			exit(EXIT_FAILURE);
		}

		D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc{};
		srv_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srv_desc.Texture2D.MipLevels = texture_desc.MipLevels;
		res = self.device->CreateShaderResourceView(self.texture, &srv_desc, &self.texture_resource_view);
		if (FAILED(res))
		{
			fprintf(stderr, "failed to create texture shader resource view");
			exit(EXIT_FAILURE);
		}

		D3D11_UNORDERED_ACCESS_VIEW_DESC uav_desc{};
		uav_desc.Format = texture_desc.Format;
		uav_desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
		uav_desc.Texture2D.MipSlice = 0;
		res = self.device->CreateUnorderedAccessView(self.texture, &uav_desc, &self.texture_unordered_view);
		if (FAILED(res))
		{
			fprintf(stderr, "failed to create texture unordered access view");
			exit(EXIT_FAILURE);
		}

		D3D11_SAMPLER_DESC sampler_desc{};
		sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
		sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		sampler_desc.MipLODBias = 0;
		sampler_desc.MaxAnisotropy = 1;
		sampler_desc.ComparisonFunc = D3D11_COMPARISON_LESS;
		sampler_desc.MinLOD = 0;
		sampler_desc.MaxLOD = D3D11_FLOAT32_MAX;
		res = self.device->CreateSamplerState(&sampler_desc, &self.texture_sampler);
		if (FAILED(res))
		{
			fprintf(stderr, "failed to create sampler state");
			exit(EXIT_FAILURE);
		}
	}

	// create raytracy shader
	{
		ID3D10Blob* error = nullptr;
		UINT compile_flags = 0;
		#if defined(_DEBUG)
			compile_flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
		#endif

		ID3D10Blob* shader_blob = nullptr;
		auto res = D3DCompile(RAYTRACE_COMPUTE_SHADER, strlen(RAYTRACE_COMPUTE_SHADER), NULL, NULL, NULL, "main", "cs_5_0", compile_flags, 0, &shader_blob, &error);
		if (FAILED(res))
		{
			fprintf(stderr, "failed to compile shader, %s\n", (char*)error->GetBufferPointer());
			exit(EXIT_FAILURE);
		}

		res = self.device->CreateComputeShader(
			shader_blob->GetBufferPointer(),
			shader_blob->GetBufferSize(),
			NULL,
			&self.raytrace_compute_shader
		);
		if (FAILED(res))
		{
			fprintf(stderr, "failed to create compute shader\n");
			exit(EXIT_FAILURE);
		}

		shader_blob->Release();
	}


	auto world = random_scene();

	// create sphere buffer
	{
		D3D11_BUFFER_DESC buffer_desc{};
		buffer_desc.ByteWidth = world.spheres.size() * sizeof(Sphere);
		buffer_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		buffer_desc.Usage = D3D11_USAGE_IMMUTABLE;
		buffer_desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		buffer_desc.StructureByteStride = sizeof(Sphere);

		D3D11_SUBRESOURCE_DATA data_desc{};
		data_desc.pSysMem = world.spheres.data();
		auto res = self.device->CreateBuffer(&buffer_desc, &data_desc, &self.raytrace_spheres_buffer);
		if (FAILED(res))
		{
			fprintf(stderr, "failed to create structured buffer");
			exit(EXIT_FAILURE);
		}

		D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc{};
		srv_desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
		srv_desc.Format = DXGI_FORMAT_UNKNOWN;
		srv_desc.BufferEx.NumElements = buffer_desc.ByteWidth / buffer_desc.StructureByteStride;
		res = self.device->CreateShaderResourceView(self.raytrace_spheres_buffer, &srv_desc, &self.raytrace_spheres_buffer_resource_view);
		if (FAILED(res))
		{
			fprintf(stderr, "failed to create spheres buffer shader resource view");
			exit(EXIT_FAILURE);
		}
	}

	// create materials buffer
	{
		D3D11_BUFFER_DESC buffer_desc{};
		buffer_desc.ByteWidth = world.materials.size() * sizeof(Material);
		buffer_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		buffer_desc.Usage = D3D11_USAGE_IMMUTABLE;
		buffer_desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		buffer_desc.StructureByteStride = sizeof(Material);

		D3D11_SUBRESOURCE_DATA data_desc{};
		data_desc.pSysMem = world.materials.data();
		auto res = self.device->CreateBuffer(&buffer_desc, &data_desc, &self.raytrace_materials_buffer);
		if (FAILED(res))
		{
			fprintf(stderr, "failed to create structured buffer");
			exit(EXIT_FAILURE);
		}

		D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc{};
		srv_desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
		srv_desc.Format = DXGI_FORMAT_UNKNOWN;
		srv_desc.BufferEx.NumElements = buffer_desc.ByteWidth / buffer_desc.StructureByteStride;
		res = self.device->CreateShaderResourceView(self.raytrace_materials_buffer, &srv_desc, &self.raytrace_materials_buffer_resource_view);
		if (FAILED(res))
		{
			fprintf(stderr, "failed to create materials buffer shader resource view");
			exit(EXIT_FAILURE);
		}
	}

	self.ready = true;
}

Renderer renderer_new()
{
	Renderer self{};

	const D3D_FEATURE_LEVEL feature_levels[] = {
		D3D_FEATURE_LEVEL_11_1,
	};

	UINT creation_flags = 0;
	#if defined(_DEBUG)
		creation_flags |= D3D11_CREATE_DEVICE_DEBUG;
	#endif

	auto res = D3D11CreateDevice(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		creation_flags,
		feature_levels,
		ARRAYSIZE(feature_levels),
		D3D11_SDK_VERSION,
		&self.device,
		nullptr,
		&self.context
	);
	if (FAILED(res))
	{
		fprintf(stderr, "failed to create device");
		exit(EXIT_FAILURE);
	}

	IDXGIDevice* dxgi_device = nullptr;
	res = self.device->QueryInterface(IID_PPV_ARGS(&dxgi_device));
	if (FAILED(res))
	{
		fprintf(stderr, "failed to get dxgi device from d3d11 device");
		exit(EXIT_FAILURE);
	}

	res = dxgi_device->GetAdapter(&self.adapter);
	if (FAILED(res))
	{
		fprintf(stderr, "failed to get adapter from dxgi device");
		exit(EXIT_FAILURE);
	}

	res = self.adapter->GetParent(IID_PPV_ARGS(&self.factory));
	if (FAILED(res))
	{
		fprintf(stderr, "failed to get DXGIFactory from adapter");
		exit(EXIT_FAILURE);
	}

	return self;
}

void renderer_free(Renderer& self)
{
	if (self.swapchain) self.swapchain->Release();
	if (self.render_target_view) self.render_target_view->Release();
	if (self.screen_rect_vertices) self.screen_rect_vertices->Release();
	if (self.compiled_vs_shader) self.compiled_vs_shader->Release();
	if (self.screen_rect_vertex_shader) self.screen_rect_vertex_shader->Release();
	if (self.screen_rect_pixel_shader) self.screen_rect_pixel_shader->Release();
	if (self.screen_rect_rasterizer_state) self.screen_rect_rasterizer_state->Release();
	if (self.screen_rect_input_layout) self.screen_rect_input_layout->Release();
	if (self.texture) self.texture->Release();
	if (self.texture_resource_view) self.texture_resource_view->Release();
	if (self.texture_unordered_view) self.texture_unordered_view->Release();
	if (self.texture_sampler) self.texture_sampler->Release();
	if (self.raytrace_compute_shader) self.raytrace_compute_shader->Release();
	if (self.raytrace_spheres_buffer) self.raytrace_spheres_buffer->Release();
	if (self.raytrace_spheres_buffer_resource_view) self.raytrace_spheres_buffer_resource_view->Release();
	if (self.raytrace_materials_buffer) self.raytrace_materials_buffer->Release();
	if (self.raytrace_materials_buffer_resource_view) self.raytrace_materials_buffer_resource_view->Release();
	self.context->Release();
	self.device->Release();
	self.adapter->Release();
	self.factory->Release();
}

// Note: a global so that we can call it from the callback, I'm lazy
Renderer renderer;

LRESULT CALLBACK _window_callback(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch (msg)
	{
	case WM_SIZE:
		renderer_draw(renderer);
		break;
	case WM_CLOSE:
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return ::DefWindowProc(hwnd, msg, wparam, lparam);
	}
	return 0;
}

void window_register_class(const char* window_class)
{
	WNDCLASSEX wc{};
	ZeroMemory(&wc, sizeof(wc));
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = _window_callback;
	wc.hInstance = NULL;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wc.lpszClassName = window_class;
	RegisterClassEx(&wc);
}

Window window_new(int width, int height, const char* title)
{
	Window self{};
	self.width = width;
	self.height = height;
	self.title = title;

	RECT wr{};
	wr.right = LONG(self.width);
	wr.bottom = LONG(self.height);

	self.handle = CreateWindowEx(
		NULL,
		WINDOW_CLASS,
		self.title,
		WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX & ~WS_THICKFRAME,
		100,
		100,
		wr.right - wr.left,
		wr.bottom - wr.top,
		NULL,
		NULL,
		NULL,
		NULL
	);
	if (self.handle == INVALID_HANDLE_VALUE)
	{
		fprintf(stderr, "failed to create window");
		exit(EXIT_FAILURE);
	}

	self.hdc = GetDC(self.handle);
	ShowWindow(self.handle, SW_SHOW);
	SetForegroundWindow(self.handle);
	SetFocus(self.handle);

	return self;
}

void window_free(Window& self)
{
	ReleaseDC(self.handle, self.hdc);
	DestroyWindow(self.handle);
}

int main(int argc, char** argv)
{
	renderer = renderer_new();

	window_register_class(WINDOW_CLASS);
	auto window = window_new(WINDOW_DEFAULT_WIDTH, WINDOW_DEFAULT_HEIGHT, "RTOW");

	renderer_setup_resources(renderer, window);

	MSG msg{};
	ZeroMemory(&msg, sizeof(msg));
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			renderer_draw(renderer);
		}
	}

	window_free(window);
	renderer_free(renderer);
	return 0;
}
