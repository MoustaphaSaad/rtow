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
			var c Color
			c.E[0] = float64(i) / (image_width - 1)
			c.E[1] = float64(j) / (image_height - 1)
			c.E[2] = 0.25
			c.Write(os.Stdout)
		}
	}

	fmt.Fprintf(os.Stderr, "\nDone.\n")
	fmt.Fprintf(os.Stderr, "Elapsed time: %v\n", time.Since(start))
}