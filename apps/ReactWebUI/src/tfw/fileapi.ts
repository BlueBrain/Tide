export default {
    saveAs(blob: Blob, fileName: string) {
        const url = window.URL.createObjectURL(blob);
        const anchorElem = document.createElement("a");

        anchorElem.style.display = "none";
        anchorElem.href = url;
        anchorElem.download = fileName;

        document.body.appendChild(anchorElem);
        anchorElem.click();

        document.body.removeChild(anchorElem);

        // On Edge, revokeObjectURL should be called only after
        // a.click() has completed, atleast on EdgeHTML 15.15048
        setTimeout(function() {
            window.URL.revokeObjectURL(url);
        }, 5000);
    }
}
