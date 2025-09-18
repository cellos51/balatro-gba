# Balatro-GBA

[![Build Status](https://img.shields.io/github/actions/workflow/status/cellos51/balatro-gba/build_ci_workflow.yml?style=flat&logo=github&branch=main&label=Builds&labelColor=gray&color=default&v=1)](https://github.com/cellos51/balatro-gba/actions)
[![Open Issues](https://custom-icon-badges.demolab.com/github/issues/cellos51/balatro-gba?logo=bug&style=flat&label=Issues&labelColor=gray&color=red&v=2)](https://github.com/cellos51/balatro-gba/issues)
[![Pull Requests](https://custom-icon-badges.demolab.com/github/issues-pr/cellos51/balatro-gba?logo=git-pull-request&style=flat&label=Pull%20Requests&labelColor=gray&color=indigo&v=3)](https://github.com/cellos51/balatro-gba/pulls)

This is an attempt to recreate the game **'Balatro'** as accurately as possible, including all of the visual effects that make Balatro feel satisfying to play.
This **tech-demo/proof of concept** is strictly limited in content to a minimal version of Balatro and will **NOT** recreate the full game. **This version is intended for people who already own and know how the official full game works.** Please refer to the Balatro Wiki if you need help understanding certain mechanics or abilities.

<a href="https://balatrowiki.org/">
  <img src="https://custom-icon-badges.demolab.com/badge/Balatro%20Wiki-194c84?logo=bigjoker&logoColor=fff" alt="Balatro Wiki" width="155">
</a>

### Disclaimer: This project is NOT endorsed by or affiliated with Playstack or LocalThunk
#### This is a non-profit community fan project solely aimed to recreate a minimal version of Balatro on the Game Boy Advance as a tribute to the full Balatro and is not intended to infringe or draw sales away from the full game's release or any of the established works by Playstack and LocalThunk.
#### All rights are reserved to their respective holders. 

### Please buy the official full version from these sources below:
[![Balatro on Steam](https://custom-icon-badges.demolab.com/badge/Balatro%20on%20Steam-194c84?logo=steam&logoColor=fff)](https://store.steampowered.com/app/2379780/Balatro/)
[![Balatro on Google Play](https://custom-icon-badges.demolab.com/badge/Balatro%20on%20Google%20Play-414141?logo=Google-play&logoColor=fff)](https://play.google.com/store/apps/details?id=com.playstack.balatro.android)
[![Balatro on Apple App Store](https://custom-icon-badges.demolab.com/badge/Balatro%20on%20Apple%20App%20Store-0D96F6?logo=app-store&logoColor=fff)](https://apps.apple.com/us/app/balatro/id6502453075)
[![Balatro on Nintendo eShop](https://custom-icon-badges.demolab.com/badge/Balatro%20on%20Nintendo%20eShop-e60012?logo=nintendo&logoColor=fff)](https://www.nintendo.com/us/store/products/balatro-switch/)
[![Balatro on PlayStation Store](https://custom-icon-badges.demolab.com/badge/Balatro%20on%20PlayStation%20Store-006FCD?logo=PlayStation&logoColor=fff)](https://store.playstation.com/en-us/concept/10010334)
[![Balatro on Xbox](https://custom-icon-badges.demolab.com/badge/Balatro%20on%20Xbox-107C10.svg?logo=xbox&logoColor=white)](https://www.xbox.com/en-US/games/store/balatro/9PK087LNGJC5)
[![Balatro on Humble Bundle](https://img.shields.io/badge/Balatro%20on%20Humble%20Bundle-%23494F5C.svg?logo=HumbleBundle&logoColor=white)](https://www.humblebundle.com/store/balatro?srsltid=AfmBOoqS2De8T4kizzWxJS1pbvQosJ_bYCl4qvC6LA1YLPAh4sZ8vJqO)

<!-- The Gif is a little blurry but I think it looks fine -->
<img src="example.gif" alt="Example GIF" width="800">

### Controls: 
(A: Pick Card/Make Selections)

(B: Deselect All Cards) 

(L: Sell Joker)

(R: Sort Suit/Rank)

(D-Pad: Navigation) 
# **Build Instructions:**

## **-Docker-**
A docker compose file is provided to build this project. 

1.) Install [docker desktop](https://docs.docker.com/compose/install/). 

2.) Open a terminal to this projects directory:
- On **linux** run `UID=$(id -u) GID=$(id -g) docker compose up`
- On **windows** run `docker compose up`

Docker will build the project and the ROM will be in the same location as step 7 describes below.

## **-Apple Container-**
If for some reason you want to use Apple Container to build this project, you can also do so!

1.) Install `container` from the releases in their [GitHub page](https://github.com/apple/container/releases)
2.) Start the container system with `container system start`
3.) Run `container build --tag balatro-builder --file Dockerfile .`
4.) Run `container run --volume $(pwd):./balatro --name balatro --rm balatro-builder`
5.) (Optional) if you want to remove the image from container, run `container image delete balatro-builder`

After this, the `balatro.gba` file should be in the build folder

## **-Windows-**
Video Tutorial: https://youtu.be/72Zzo1VDYzQ?si=UDmEdbST1Cx1zZV2
### With `Git` (not required)
1.) Install `devkitPro` from https://github.com/devkitPro/installer by downloading the latest release, unzipping the archive, and then running the installer. You only need to install the tools for the GBA development so ensure you have it selected and install it to the default directory. You may need to create a temp folder for the project.

2.) Search for `MSys2` in the Start Menu and open it.

3.) Install `Git` by typing this command: `pacman -S git` if you don't have it already installed

4.) Clone the project by putting `git clone https://github.com/cellos51/balatro-gba.git` in the MSys2 window.

5.) Use `cd` and drag the new folder into the window to add the file path and press Enter.

6.) Type `make` into the window and press Enter to start building the rom.

7.) After it completes, navigate through the `build` directory in the project folder and look for `balatro-gba.gba` and load it on an emulator or flashcart.
### Without `Git`
Disregard Steps 3-4 and instead click the green code button on the main repository page and press `Download Zip`. Unzip the folder and place it wherever you like. Then continue from Step 5.
## **Common Issues:**

#### 1. **When I run `make` it errors out and won't compile!**
- Move the project folder to your desktop and then `cd` to it by dragging the folder into the terminal window. This error could be caused by a bad file path, placing the folder on the desktop is a failsafe.

#### 2. **I can't find the compiled rom!**
- Look for a folder called `build` inside the project folder and then find `balatro-gba.gba`.

#### 3. **The Game won't start!**
- Try a different emulator or if you are using original hardware, make sure the rom is not corrupted and try a different flashcart or SD Card. If this does not work, open an issue on the Github page because a recent commit may have broke the game.

#### 4. **It says I don't have `Git` or `Make` installed!**
- Use `pacman -S git` (not required) or `pacman -S make` although make should already be installed if you followed the instructions correctly.
