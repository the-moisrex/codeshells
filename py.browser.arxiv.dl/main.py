#!/usr/bin/env python3

"""py.conda

{PythonDesc}
"""

# pip install pytest-playwright
# playwright install firefox
# playwright install-deps # it gives errors

from playwright.sync_api import sync_playwright
from urllib.request import urlretrieve
import os


def main():
    pw = sync_playwright().start()
    browser = pw.firefox.launch(
            headless=False,
            slow_mo=100)
    page = browser.new_page()
    page.goto("https://arxiv.org/search")
    page.get_by_placeholder("Search term...").fill("neural network")
    # page.keyboard.press("Enter")
    page.get_by_role("button").get_by_text("Search").nth(1).click()
    links = page.locator("xpath=//a[contains(@href, 'arxiv.org/pdf')]").all()

    for link in links:
        url = link.get_attribute("href")
        urlretrieve(url, "./" + url[-5:] + ".pdf")

    # print(page.title())
    # page.screenshot(path="screenshot.png")
    # os.system("xdg-open screenshot.png")


if __name__ == "__main__":
    main()
