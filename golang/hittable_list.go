package main

type HittableList struct {
	spheres []Sphere
}

func (list *HittableList) Add(h Sphere) {
	list.spheres = append(list.spheres, h)
}

func (list *HittableList) Clear() {
	list.spheres = make([]Sphere, 0)
}

func (list *HittableList) Hit(r Ray, tMin, tMax Scalar) (rec HitRecord, hit bool) {
	hit = false
	closestSoFar := tMax

	for i := range list.spheres {
		if v_rec, v_hit := list.spheres[i].Hit(r, tMin, closestSoFar); v_hit {
			hit = true
			closestSoFar = v_rec.T
			rec = v_rec
		}
	}

	return
}
