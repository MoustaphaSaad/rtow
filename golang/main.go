package main

import (
	"flag"
	"fmt"
	"log"
	"math/rand"
	"os"
	"runtime"
	"runtime/pprof"
	"sync"
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

func rayColor(r Ray, world *HittableList, depth int, stat *RaytraceStat) Color {
	if depth <= 0 {
		return Color{}
	}

	if rec := world.Hit(r, 0.001, infinity); rec != nil {
		m := &world.materials[rec.MaterialIndex]
		if attenuation, scattered := m.Scatter(r, rec); scattered != nil {
			stat.bounces++
			return attenuation.HMul(rayColor(*scattered, world, depth - 1, stat))
		}
		return Color{}
	}
	unitDirection := r.Dir.UnitVector()
	t := 0.5 * (unitDirection.Y + 1)
	startColor := Color{1, 1, 1}
	endColor := Color{0.5, 0.7, 1}
	return startColor.Mul(1 - t).Add(endColor.Mul(t))
}

func randomScene() HittableList {
	var world HittableList

	groundMaterial := world.AddMaterial(Lambertian(Color{0.5, 0.5, 0.5}))
	world.AddSphere(Sphere{Point3{0, -1000, 0}, 1000, groundMaterial})

	for a := -11; a < 11; a++ {
		for b := -11; b < 11; b++ {
			chooseMat := RandomDouble();
			center := Point3{Scalar(a) + 0.9*RandomDouble(), 0.2, Scalar(b) + 0.9*RandomDouble()}

			if center.Sub(Point3{4, 0.2, 0}).Length() > 0.9 {
				var sphereMaterial int

				if chooseMat < 0.8 {
					albedo := RandomVec3()
					sphereMaterial = world.AddMaterial(Lambertian(albedo))
					world.AddSphere(Sphere{center, 0.2, sphereMaterial})
				} else if chooseMat < 0.95 {
					albedo := RandomVec3InRange(0.5, 1)
					fuzz := RandomDoubleInRange(0, 0.5)
					sphereMaterial = world.AddMaterial(Metal(albedo, fuzz))
					world.AddSphere(Sphere{center, 0.2, sphereMaterial})
				} else {
					sphereMaterial = world.AddMaterial(Dielectric(1.5))
					world.AddSphere(Sphere{center, 0.2, sphereMaterial})
				}
			}
		}
	}

	material1 := world.AddMaterial(Dielectric(1.5))
	world.AddSphere(Sphere{Point3{0, 1, 0}, 1.0, material1})

	material2 := world.AddMaterial(Lambertian(Color{0.4, 0.2, 0.1}))
	world.AddSphere(Sphere{Point3{-4, 1, 0}, 1.0, material2})

	material3 := world.AddMaterial(Metal(Color{0.7, 0.6, 0.5}, 0))
	world.AddSphere(Sphere{Point3{4, 1, 0}, 1.0, material3})

	world.PrepareSOA()

	return world
}

type RaytraceStat struct {
	ray_count, bounces int
}

type ImageTile struct {
	StartX, StartY int
	EndX, EndY int
}

type TileTask struct {
	img *Image
	cam *Camera
	world *HittableList
	tile ImageTile
	samplesPerPixel int
	maxDepth int
}

func traceTile(c <-chan TileTask, wg *sync.WaitGroup, stat *RaytraceStat) {
	for j := range c {
		for y := j.tile.StartY; y < j.tile.EndY; y++ {
			for x := j.tile.StartX; x < j.tile.EndX; x++ {
				pixelColor := Color{0, 0, 0}
				for s := 0; s < j.samplesPerPixel; s++ {
					u := (Scalar(x) + Rand()) / Scalar(j.img.width - 1)
					v := (Scalar(y) + Rand()) / Scalar(j.img.height - 1)
					r := j.cam.ray(u, v)
					pixelColor = pixelColor.Add(rayColor(r, j.world, j.maxDepth, stat))
					stat.ray_count++
				}

				scale := 1.0 / Scalar(j.samplesPerPixel)
				pixelColor.X = Sqrt(scale * pixelColor.X)
				pixelColor.Y = Sqrt(scale * pixelColor.Y)
				pixelColor.Z = Sqrt(scale * pixelColor.Z)
				j.img.SetPixel(x, y, pixelColor)
			}
		}
	}
	wg.Done()
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

	// keep the randomness in control
	rand.Seed(42)

	start := time.Now()

	// Image
	var aspectRatio = Scalar(16.0) / Scalar(9.0)
	var imageWidth = 640
	var imageHeight = int(Scalar(imageWidth) / aspectRatio)
	var samplesPerPixel = 10
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

	img := NewImage(imageWidth, imageHeight)

	// launch as many workers as we have cores
	var wg sync.WaitGroup
	wg.Add(runtime.NumCPU())
	workChan := make(chan TileTask, runtime.NumCPU() * 2)
	worker_stat := make([]RaytraceStat, runtime.NumCPU())
	for i := 0; i < runtime.NumCPU(); i++ {
		go traceTile(workChan, &wg, &worker_stat[i])
	}

	tileSizeX := 16
	tileCountX := 1 + ((img.width - 1) / tileSizeX)
	tileSizeY := 16
	tileCountY := 1 + ((img.height - 1) / tileSizeY)

	tileTotalCount := tileCountX * tileCountY

	t := TileTask {
		img: img,
		cam: &cam,
		world: &world,
		samplesPerPixel: samplesPerPixel,
		maxDepth: maxDepth,
	}

	for x := 0; x < tileCountX; x++ {
		t.tile.StartX = x * tileSizeX
		t.tile.EndX = t.tile.StartX + tileSizeX
		if t.tile.EndX > img.width { t.tile.EndX = img.width }

		for y := 0; y < tileCountY; y++ {
			t.tile.StartY = y * tileSizeY
			t.tile.EndY = t.tile.StartY + tileSizeY
			if t.tile.EndY > img.height { t.tile.EndY = img.height }
			fmt.Fprintf(os.Stderr, "\rTile remaining: %v       ", tileTotalCount)
			workChan <- t
			tileTotalCount--
		}
	}

	fmt.Fprintf(os.Stderr, "\rTile remaining: %v       ", tileTotalCount)

	close(workChan)
	wg.Wait()
	elapsedTime := time.Since(start)

	img.Write()

	totalStat := RaytraceStat{}
	for _, stat := range worker_stat {
		totalStat.ray_count += stat.ray_count
		totalStat.bounces += stat.bounces
	}

	fmt.Fprintf(os.Stderr, "\nDone.\n")
	fmt.Fprintf(os.Stderr, "Elapsed time: %v\n", elapsedTime)
	fmt.Fprintf(os.Stderr, "Total Rays: %.2f MRays, Bounces: %.2f MRays\n", float64(totalStat.ray_count) / 1000000, float64(totalStat.bounces) / 1000000)
	raysCount := totalStat.ray_count + totalStat.bounces
	fmt.Fprintf(os.Stderr, "Ray Per Sec: %.2f MRays/Second\n", (float64(raysCount) / elapsedTime.Seconds()) / 1000000)
}