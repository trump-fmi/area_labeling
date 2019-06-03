function data_input() {
    console.log("Input Test!");

    let input = document.getElementById("label_input");
    let files = input.files;
    if (files.length != 1) {
        console.log(`The number of selected files (${files.length}) is invalid!`);
        return;
    }

    let reader = new FileReader();
    reader.onload = function (e) {
        let d = read_data(e.target.result);
        refresh(d);
    }
    reader.readAsText(files[0]);
}

function read_data(data) {
    let jData = JSON.parse(data);

    let res = [];

    jData.forEach(element => {
        parseElem(element, res);
    });

    return jData;
}

function parseElem(json, res) {
    switch (json.type) {
        case "Feature":
            if (json.geometry.type === "Point") {
                res.push(parsePoint(json));
            } else {
                res.push(parseArea(json));
            }
            break;
        case "FeatureCollection":
            res.push(parseAreaPOI);
            break;
        default:
            console.log(`Unknown geojson type: ${json.type}`);
    }
}

function parsePOI(json) {
    return new PointOfInterest(json.properties.label, json.geometry.coordinates, 120, json.properties.label.font.size);
}

function parseArea(json) {
    return new Area(
        json.geometry.coordinates[0],
        json.geometry.coordinates.slice(1),
        new AreaLabel(
            json.properties.label.label,
            json.properties.label.baseline.center,
            json.properties.label.baseline.radius,
            json.properties.label.font.size,
            json.properties.label.baseline.from,
            json.properties.label.baseline.to
        )
    );
}

function parseAreaPOI(json) {
    let poi = parsePointLabel(json.features.poi);
    let area = parseArea(json.features.area);

    return new AreaPOI(area, poi);
}