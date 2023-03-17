package main

import (
	"fmt"
	"math"
	"math/rand"
	"os"
	"time"
)

func hitSphere(center Point3, radius float64, r Ray) float64 {
	// sphere around arbitrary center equation is
	// P is a point in 3D space
	// (P - center)^2 = radius^2 -> (P.x - center.x)^2 + (P.y - center.y)^2 + (P.z - center.z)^2 = radius^2
	// if you replace P with (ray.origin + t * ray.direction) and simplify
	// you get a quadratic equation
	// t^2 * (ray.direction.x^2 + ray.direction.y^2 + ray.direction.z^2) +
	// 2t * (ray.direction.x * (ray.origin.x - center.x) + ray.direction.y * (ray.origin.y - center.y) + ray.direction.z * (ray.origin.z - center.z)) +
	// (ray.origin.x - center.x)^2 + (ray.origin.y - center.y)^2 + (ray.origin.z - center.z)^2 - radius^2 = 0
	// which means that
	// a = dot(ray.direction, ray.direction)
	// b = dot((ray.origin - center), ray.direction)
	// c = dot((ray.origin - center), (ray.origin - center)) - radius^2
	// and using the quadratic equation formula you have the discriminant = b^2 - 4ac, if it's positive we have 2 solutions
	// if it's 0 we have one, if it's negative we have no solution

	a := r.Dir.LengthSquared()
	oc := r.Orig.Sub(center)
	halfB := oc.Dot(r.Dir)
	c := oc.LengthSquared() - radius * radius
	discriminant := halfB * halfB - a * c
	if discriminant < 0 {
		return -1
	} else {
		return (-halfB - math.Sqrt(discriminant)) / a
	}
}

var infinity = math.Inf(1)
const pi = 3.1415926535897932385

func degressToRadians(degrees float64) float64 {
	return degrees * pi / 180
}

func rayColor(r Ray, world Hittable, depth int) Color {
	if depth <= 0 {
		return Color{}
	}

	if rec, hit := world.Hit(r, 0.001, infinity); hit {
		if res, scattered := rec.Mat.Scatter(r, rec); scattered {
			return res.Attenuation.HMul(rayColor(res.Scattered, world, depth - 1))
		}
		return Color{}
	}
	unitDirection := r.Dir.UnitVector()
	t := 0.5 * (unitDirection.Y() + 1)
	startColor := Color{1, 1, 1}
	endColor := Color{0.5, 0.7, 1}
	return startColor.Mul(1 - t).Add(endColor.Mul(t))
}

func main() {
	start := time.Now()

	// Image
	var aspectRatio = 16.0 / 9.0
	var imageWidth = 400
	var imageHeight = int(float64(imageWidth) / aspectRatio)
	var samplesPerPixel = 100
	var raysCount = imageWidth * imageHeight * samplesPerPixel
	var maxDepth = 50

	// World
	var world HittableList
	materialGround := Lambertian{Albedo: Color{0.8, 0.8, 0.0}}
	materialCenter := Lambertian{Albedo: Color{0.1, 0.2, 0.5}}
	materialLeft := Dielectric{IndexOfRefraction: 1.5}
	materialRight := Metal{Albedo: Color{0.8, 0.6, 0.2}, Fuzz: 1}

	world.Add(Sphere{Center: Point3{0, -100.5, -1}, Radius: 100, Mat: materialGround})
	world.Add(Sphere{Center: Point3{0, 0, -1}, Radius: 0.5, Mat: materialCenter})
	world.Add(Sphere{Center: Point3{-1, 0, -1}, Radius: -0.4, Mat: materialLeft})
	world.Add(Sphere{Center: Point3{1, 0, -1}, Radius: 0.5, Mat: materialRight})

	// Camera
	cam := NewCamera()

	fmt.Printf("P3\n%v %v\n255\n", imageWidth, imageHeight)

	var pixelOnly time.Duration

	for j := imageHeight - 1; j >= 0; j-- {
		fmt.Fprintf(os.Stderr, "\rElapsed time: %v, ", time.Since(start))
		fmt.Fprintf(os.Stderr, "Scanlines remaining: %v ", j)
		for i := 0; i < imageWidth; i++ {
			start := time.Now()
			pixelColor := Color{0, 0, 0}
			for s := 0; s < samplesPerPixel; s++ {
				u := (float64(i) + rand.Float64()) / float64(imageWidth - 1)
				v := (float64(j) + rand.Float64()) / float64(imageHeight - 1)
				r := cam.ray(u, v)
				pixelColor = pixelColor.Add(rayColor(r, world, maxDepth))
			}
			pixelOnly += time.Since(start)

			pixelColor.Write(os.Stdout, float64(samplesPerPixel))
		}
	}

	fmt.Fprintf(os.Stderr, "\nDone.\n")
	fmt.Fprintf(os.Stderr, "Elapsed time: %v\n", time.Since(start))
	fmt.Fprintf(os.Stderr, "Pixel time: %v\n", pixelOnly)
	fmt.Fprintf(os.Stderr, "Ray Per Sec: %.2f MRays/Second\n", (float64(raysCount) / pixelOnly.Seconds()) / 1000000)
}