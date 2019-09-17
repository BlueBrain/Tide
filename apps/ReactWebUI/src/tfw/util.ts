export default {
    padNumber(value: number, size: number = 6, fillWith: string = '0'): string {
        let text = `${value}`;
        while (text.length < size) {
            text = fillWith.charAt(0) + text;
        }
        return text;
    },

    clamp(value: number, min: number = 0, max: number = 1) {
        if (value < min) return min;
        if (value > max) return max;
        return value;
    },

    loadTextFromURL(url: string): Promise<string> {
        return new Promise((resolve, reject) => {
            fetch(url)
                .then((response: Response) => {
                    if (!response.ok) {
                        reject(`Error ${response.status}: ${response.statusText}!`);
                    }
                    return response.text();
                })
                .then(resolve)
                .catch(reject);
        });
    },

    loadJsonFromURL(url: string): Promise<any> {
        return new Promise((resolve, reject) => {
            fetch(url)
                .then((response: Response) => {
                    return response.json();
                })
                .then(resolve)
                .catch(reject);
        });
    }
}
