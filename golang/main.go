package main

import (
	"fmt"
	"os"
	"time"
)

func main() {
	start := time.Now()
	const image_width = 256
	const image_height = 256

	fmt.Printf("P3\n%v %v\n255\n", image_width, image_height)

	for j := image_height - 1; j >= 0; j-- {
		fmt.Fprintf(os.Stderr, "\rElapsed time: %v, ", time.Since(start))
		fmt.Fprintf(os.Stderr, "Scanlines remaining: %v ", j)
		for i := 0; i < image_width; i++ {
			r := float64(i) / (image_width - 1)
			g := float64(j) / (image_height - 1)
			b := 0.25

			ir := int(255.999 * r)
			ig := int(255.999 * g)
			ib := int(255.999 * b)

			fmt.Printf("%v %v %v\n", ir, ig, ib)
		}
	}

	fmt.Fprintf(os.Stderr, "\nDone.\n")
	fmt.Fprintf(os.Stderr, "Elapsed time: %v\n", time.Since(start))
}