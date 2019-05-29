class AreaLabel {
    constructor(label, center, radInner, radOuter, angleStart, angleEnd) {
        this.label = label;
        this.center = center;
        this.radInner = radInner;
        this.radOuter = radOuter;
        this.angleStart = angleStart;
        this.angleEnd = angleEnd;
    }

    draw(canvas) {
        canvas.drawCurvedLabel([this.center, [this.radInner, this.radOuter], [this.angleStart, this.angleEnd], this.label]);
    }
}