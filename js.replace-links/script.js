
function replURL(el, attr) {
    const url = new URL(el.getAttribute(attr));
    url.protocol = url.protocol == "http:" ? "https:" : url.protocol;
    el.setAttribute(attr, url.toString());
    console.log(url.toString());
}

function findLinks() {
    document.querySelectorAll('a').forEach(link => replURL(link, 'href'));
    document.querySelectorAll('img').forEach(link => replURL(link, 'src'));
}

window.onload = findLinks;

console.log("<a href='https://example.com/'>Link</a>")
