package main

import (
	"fmt"
	"os"
	"time"
)

func hitSphere(center Point3, radius float64, r Ray) bool {
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

	a := r.Dir.Dot(r.Dir)
	oc := r.Orig.Sub(center)
	b := 2 * oc.Dot(r.Dir)
	c := oc.Dot(oc) - radius * radius
	discriminant := b * b - 4 * a * c
	return discriminant > 0
}

func rayColor(r Ray) Color {
	if hitSphere(Point3{0, 0, -1}, 0.5, r) {
		return Color{1, 0, 0}
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

	// Camera
	var viewportHeight = 2.0
	var viewportWidth = aspectRatio * viewportHeight
	var focalLength = 1.0

	var origin = Point3{0, 0, 0}
	var horizontal = Vec3{viewportWidth, 0, 0}
	var vertical = Vec3{0, viewportHeight, 0}
	var lowerLeftCorner = origin.Sub(horizontal.Div(2)).Sub(vertical.Div(2)).Sub(Vec3{0, 0, focalLength})

	fmt.Printf("P3\n%v %v\n255\n", imageWidth, imageHeight)

	for j := imageHeight - 1; j >= 0; j-- {
		fmt.Fprintf(os.Stderr, "\rElapsed time: %v, ", time.Since(start))
		fmt.Fprintf(os.Stderr, "Scanlines remaining: %v ", j)
		for i := 0; i < imageWidth; i++ {
			u := float64(i) / float64(imageWidth - 1)
			v := float64(j) / float64(imageHeight - 1)

			r := Ray{
				origin,
				lowerLeftCorner.Add(horizontal.Mul(u)).Add(vertical.Mul(v)).Sub(origin),
			}
			pixelColor := rayColor(r)
			pixelColor.Write(os.Stdout)
		}
	}

	fmt.Fprintf(os.Stderr, "\nDone.\n")
	fmt.Fprintf(os.Stderr, "Elapsed time: %v\n", time.Since(start))
}