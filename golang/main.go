package main

import (
	"flag"
	"fmt"
	"log"
	"os"
	"runtime/pprof"
	"time"
)

func hitSphere(center Point3, radius Scalar, r Ray) Scalar {
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
		return (-halfB - Sqrt(discriminant)) / a
	}
}

const pi = 3.1415926535897932385

func degressToRadians(degrees Scalar) Scalar {
	return degrees * pi / 180
}

func rayColor(r Ray, world *HittableList, depth int) Color {
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

func randomScene() HittableList {
	var world HittableList

	groundMaterial := Lambertian{Color{0.5, 0.5, 0.5}}
	world.Add(Sphere{Point3{0, -1000, 0}, 1000, groundMaterial})

	for a := -11; a < 11; a++ {
		for b := -11; b < 11; b++ {
			chooseMat := RandomDouble();
			center := Point3{Scalar(a) + 0.9*RandomDouble(), 0.2, Scalar(b) + 0.9*RandomDouble()}

			if center.Sub(Point3{4, 0.2, 0}).Length() > 0.9 {
				var sphereMaterial Material

				if chooseMat < 0.8 {
					albedo := RandomVec3()
					sphereMaterial = Lambertian{albedo}
					world.Add(Sphere{center, 0.2, sphereMaterial})
				} else if chooseMat < 0.95 {
					albedo := RandomVec3InRange(0.5, 1)
					fuzz := RandomDoubleInRange(0, 0.5)
					sphereMaterial = Metal{albedo, fuzz}
					world.Add(Sphere{center, 0.2, sphereMaterial})
				} else {
					sphereMaterial = Dielectric{1.5}
					world.Add(Sphere{center, 0.2, sphereMaterial})
				}
			}
		}
	}

	material1 := Dielectric{1.5}
	world.Add(Sphere{Point3{0, 1, 0}, 1.0, material1})

	material2 := Lambertian{Color{0.4, 0.2, 0.1}}
	world.Add(Sphere{Point3{-4, 1, 0}, 1.0, material2})

	material3 := Metal{Color{0.7, 0.6, 0.5}, 0}
	world.Add(Sphere{Point3{4, 1, 0}, 1.0, material3})

	return world
}

var cpuprofile = flag.String("cpuprofile", "", "write cpu profile to file")

func main() {
	flag.Parse()
	if *cpuprofile != "" {
		f, err := os.Create(*cpuprofile)
		if err != nil {
			log.Fatal(err)
		}
		pprof.StartCPUProfile(f)
		defer pprof.StopCPUProfile()
	}

	start := time.Now()

	// Image
	var aspectRatio = Scalar(16.0) / Scalar(9.0)
	var imageWidth = 640
	var imageHeight = int(Scalar(imageWidth) / aspectRatio)
	var samplesPerPixel = 10
	var raysCount = imageWidth * imageHeight * samplesPerPixel
	var maxDepth = 50

	// World
	world := randomScene()

	// Camera
	lookFrom := Point3{13, 2, 3}
	lookAt := Point3{0, 0, 0}
	vUp := Vec3{0, 1, 0}
	distToFocus := Scalar(10.0)
	aperture := Scalar(0.1)
	cam := NewCamera(lookFrom, lookAt, vUp, 20, aspectRatio, aperture, distToFocus)

	fmt.Printf("P3\n%v %v\n255\n", imageWidth, imageHeight)

	var pixelOnly time.Duration

	for j := imageHeight - 1; j >= 0; j-- {
		fmt.Fprintf(os.Stderr, "\rElapsed time: %v, ", time.Since(start))
		fmt.Fprintf(os.Stderr, "Scanlines remaining: %v ", j)
		for i := 0; i < imageWidth; i++ {
			start := time.Now()
			pixelColor := Color{0, 0, 0}
			for s := 0; s < samplesPerPixel; s++ {
				u := (Scalar(i) + Rand()) / Scalar(imageWidth - 1)
				v := (Scalar(j) + Rand()) / Scalar(imageHeight - 1)
				r := cam.ray(u, v)
				pixelColor = pixelColor.Add(rayColor(r, &world, maxDepth))
			}
			pixelOnly += time.Since(start)

			pixelColor.Write(os.Stdout, Scalar(samplesPerPixel))
		}
	}

	fmt.Fprintf(os.Stderr, "\nDone.\n")
	fmt.Fprintf(os.Stderr, "Elapsed time: %v\n", time.Since(start))
	fmt.Fprintf(os.Stderr, "Pixel time: %v\n", pixelOnly)
	fmt.Fprintf(os.Stderr, "Ray Per Sec: %.2f MRays/Second\n", (float64(raysCount) / pixelOnly.Seconds()) / 1000000)
}