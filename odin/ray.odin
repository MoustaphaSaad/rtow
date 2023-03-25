package main

Ray :: struct {
	Orig: Point3,
	Dir:  Vec3,
}

ray_at :: proc(r: Ray, t: f32) -> Point3 {
	return r.Orig + r.Dir * t
}
