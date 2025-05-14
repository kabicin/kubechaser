package window

type Coord struct {
	X int32
	Y int32
}

type Window interface {
	GetOrigin() Coord
	GetDimension() Coord
	Resize(Coord, Coord)
	Hide()
	Show()
	Draw(uint32)
}
