
const languages = {
    "en": {

    },
    "zh": {
        "system": "系统",
        "screen": "屏幕",
        "controls": "控制",
        "timezone": "时区",
        "wifi": "网络",
        "weather": "天气",
        "apply": "应用",
        "done": "完成",
        "vumeter": "音量表",
        "audioinfo": "音频信息",
        "update": "固件更新",
        "screen": "屏幕",
        "screenon": "屏幕开关",
        "flipscreen": "翻转屏幕",
        "invertdisplay": "屏幕反色",
        "Import": "导入",
        "Export": "导出",
        "Add": "添加",
        "Remove": "删除",
        "Save": "保存",
        "Controls": "控制",
        "volume steps": "音量",
        "encoder acceleration": "编码器加速度",
        "save_and_reboot": "保存并重启",
        "Weather": "天气",
        "showweather": "显示天气",
        "numplaylist": "播放列表序号",
        "Playlist Editor": "播放列表编辑器",
        "volume": "音量",
        "bitinfo": "比特率",
        "rsiinfo": "信号强度",
    }
};

const $t = key => {
    return languages["zh"][key];
}

document.addEventListener("DOMContentLoaded", () => {
    document.querySelectorAll("[data-i18n]").forEach(el => {
        const id = el.getAttribute("id");
        const key = el.getAttribute("data-i18n");
        const text = el.textContent;
        el.textContent = $t(key || id || text) || text;
    });
});