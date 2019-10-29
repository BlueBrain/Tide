const tideVarsTools = {
    parse() {
        const args = {},
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
    },

    pad(txt) {
        if (!txt) return ""
        return `${txt}/`
    }
}

const args = tideVarsTools.parse()

const restUrl = `tide/${tideVarsTools.pad(args.wall)}`;
const closeImageUrl = "img/close.svg";
const focusCurtain = "focusCurtain";
const focusImageUrl = "img/focus.svg";
const fullscreenImageUrl = "img/maximize.svg";
const loadingGifUrl = "img/loading.gif";
const fullscreenCurtain = "fullscreenCurtain";
const lockImageUrl = "img/lock.svg"
const unlockImageUrl = "img/unlock.svg"
const screenOffImageUrl = "img/screenOff.svg"
const screenOnImageUrl = "img/screenOn.svg"
const stickyBezelDefaultSize = 12;
const wallOutlineWidth = 12;
const modeFocus = 1;
const modeFullscreen = 2;
const refreshInterval = 1000;
const zIndexFullscreenCurtain = 99;
const zIndexFocusCurtain = 97;
const zIndexFocus = 98;
const zIndexFullscreen = 100;
const zoomInterval = 50;
const showEffectSpeed = 200;
const maxUploadSizeMBWithoutWarning = 200;
const maxUploadSizeMB = 1024;
const MBtoB = 1048576;
