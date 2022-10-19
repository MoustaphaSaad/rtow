package main

import (
	"fmt"
	"os"
	"time"
)

func rayColor(r Ray) Color {
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