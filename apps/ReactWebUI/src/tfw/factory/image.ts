export default {
    fromArrayBuffer, fromBlob
}


function fromBlob(blob: Blob): Promise<HTMLImageElement> {
    const url = URL.createObjectURL(blob);
    const img = new Image();
    return new Promise<HTMLImageElement>(resolve => {
        img.src = url;
        // https://medium.com/dailyjs/image-loading-with-image-decode-b03652e7d2d2
        if (img.decode) {
            img.decode()
                // TODO: Figure out why decode() throws DOMException
                .then(() => resolve(img));
        } else {
            img.onload = () => resolve(img);
        }
    });
}


function fromArrayBuffer(arrayBuffer: ArrayBuffer): Promise<HTMLImageElement> {
    const blob = new Blob([arrayBuffer])
    return fromBlob(blob)
}
