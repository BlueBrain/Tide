interface IMap {
    [key: string]: string;
}

const UrlArgs = {
    parse(): IMap {
        const
            args: IMap = {},
            t = window.location.search;

        if (t.length < 2) return args;
        const items = t.substring(1).split('&');
        items.forEach((item, index) => {
            const itemWithSpacesInsteadOfPlus = item.split("+").join(" ");
            const indexOfEqual = itemWithSpacesInsteadOfPlus.indexOf("=");
            if (indexOfEqual === -1) {
                args[`${index}`] = itemWithSpacesInsteadOfPlus;
            } else {
                const key = itemWithSpacesInsteadOfPlus.substr(0, indexOfEqual);
                const val = itemWithSpacesInsteadOfPlus.substr(indexOfEqual + 1);
                args[key] = decodeURIComponent(val);
            }
        });

        return args;
    }
};

export default UrlArgs;
