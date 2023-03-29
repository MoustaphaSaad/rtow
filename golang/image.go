package main

import (
	"fmt"
)

type Image struct {
	pixels        []Color
	width, height int
}

func NewImage(w, h int) *Image {
	return &Image{
		pixels: make([]Color, w*h),
		width:  w,
		height: h,
	}
}

func (r *Image) GetPixel(x, y int) Color {
	return r.pixels[y*r.width+x]
}

func (r *Image) SetPixel(x, y int, c Color) {
	r.pixels[y*r.width+x] = c
}

func (r *Image) Write() {
	fmt.Printf("P3\n%v %v\n255\n", r.width, r.height)
	for j := r.height - 1; j >= 0; j-- {
		for i := 0; i < r.width; i++ {
			c := r.GetPixel(i, j)
			r := c.X
			g := c.Y
			b := c.Z

			fmt.Printf(
				"%v %v %v\n",
				int(256 * Clamp(r, 0.0, 0.999)),
				int(256 * Clamp(g, 0.0, 0.999)),
				int(256 * Clamp(b, 0.0, 0.999)),
			)
		}
	}
}