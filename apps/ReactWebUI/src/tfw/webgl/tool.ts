export default { endsWith, fetchAssets }


interface IAssetsOutput {
    [key: string]: string | HTMLImageElement | HTMLAudioElement;
}
/**
 * Load asset as different as image, audio, JSON and plain text.
 */
export function fetchAssets(assets: { [key: string]: string },
    onProgress: ((value: number) => void) | null = null): Promise<IAssetsOutput> {
    return new Promise(function(resolve) {
        const assetNames = Object.keys(assets);
        const count = assetNames.length;
        let done = 0;

        const result: IAssetsOutput = {};

        function next() {
            done++;
            if (typeof onProgress === 'function') {
                onProgress(done / count);
            }
            if (done >= count) resolve(result);
        }

        function onerror(key: string, ex: any) {
            console.error(`Unable to load image "${key}": ${assets[key]}`);
            console.error(ex);
            next();
        }

        function onaudioloaded(audio: HTMLAudioElement, url: string) {
            console.log("Slot for ", url);
            if (audio._loaded) return;
            audio._loaded = true;
            console.log("Loaded: ", url);
            next();
        }

        function setContent(key: string, content: any) {
            result[key] = content;
            next();
        }

        function parseText(url: string, response: any) {
            if (!response.ok) throw Error("");
            if (endsWith(url, "json")) {
                return response.json();
            } else {
                return response.text();
            }
        }

        for (const key of assetNames) {
            var url = assets[key];
            if (endsWith(url, "jpg", "png", "gif", "svg")) {
                var img = new Image();
                img.crossOrigin = "anonymous";
                result[key] = img;
                img.onload = next.bind(null, url);
                img.onerror = onerror.bind(null, key);
                img.src = url;
            }
            else if (endsWith(url, "ogg", "wav", "mp3")) {
                const audio = document.createElement("audio");
                result[key] = audio;
                const slot = onaudioloaded.bind(null, audio, url);
                audio.addEventListener("canplay", slot);
                audio.addEventListener("loadeddata", slot);
                window.setTimeout(slot, 3000);
                audio.addEventListener("error", onerror.bind(null, key));
                audio.src = url;
                console.log("Loading audio: ", url);
            }
            else {
                fetch(url)
                    .then(parseText.bind(null, url))
                    .then(setContent.bind(null, key))
                    .catch(onerror.bind(null, key));
            }
        }
    })
}


export function endsWith(text: string, ...extensions: string[]): boolean {
    const cleanText = text.trim();
    for (const extension of extensions) {
        if (cleanText.substr(cleanText.length - extension.length) === extension) return true;
    }
    return false;
}
