package main

type HittableList []Hittable

func (list HittableList) Hit(r Ray, tMin, tMax float64) (rec HitRecord, hit bool) {
	hit = false
	closestSoFar := tMax

	for _, v := range list {
		if v_rec, v_hit := v.Hit(r, tMin, closestSoFar); v_hit {
			hit = true
			closestSoFar = v_rec.T
			rec = v_rec
		}
	}

	return
}
