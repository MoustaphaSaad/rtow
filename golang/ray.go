package main

type Ray struct {
	Orig Point3
	Dir  Vec3
}

func (r Ray) At(t Scalar) Point3 {
	return r.Orig.Add(r.Dir.Mul(t))
}