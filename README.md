# CodeShells | Test/Benchmark/Prototypes

![demo](https://user-images.githubusercontent.com/12122474/149156571-dc48cde7-547a-4150-b5df-3cf6783b7976.png)


This project is written tests, benchmarks, researches, fun-things-to-know, and other random things that I wrote through out years.
Of course I don't have most of what I wrote.

All of these projects have been created with the little tool that I wrote called **codeshell**. You can find the [codeshell binary](https://github.com/the-moisrex/linux-dotfiles/blob/master/bin/codeshell) in my [linux-dotfiles](https://github.com/the-moisrex/linux-dotfiles)' bin directory.

You can create new ones with:

```bash
codeshell -t=[template] -n=[name]
# or using:
codeshell [template] [name]
# or
codeshell [template] # creates a temp directory
# or
codeshell [name]     # uses the default "simple" directory which is a C++ hello world
```

if you used the temporary directory, then you can save it with (which moves everything to `~/codeshells/` directory with the specified name):
```bash
codeshell save [new_name]
```

Replace `[template]` with one of the template directories in [code-templates directory](https://github.com/the-moisrex/linux-dotfiles/tree/master/code-templates).

Replace `[name]` with one of the above directory names or choose a new name to create one in your `~/codeshells/` directory.


### Help (`codeshell --help`)

```
Distro: arch
Usage: codeshell [template=simple] [name=...]
       codeshell help
       codeshell save   NAME         # Save the temp codeshell you're in
       codeshell remove [NAME]       # Remove the codeshell you're in or the codeshell you named
       codeshell trash  [NAME]       # Move to trash the codeshell you're in or the codeshell you named

                 -n=*|--name=*       the name of the project
                 -t=*|--template=*   the template to use
                 -g=*|-G=*           the build system to use in cmake


Flow of usage:
   Imagine you wanna try something real quick; you type:
     $ codeshell c.linux

   Now you do all your testings...
   Now you liked the tests and want to save it, you type:
     $ codeshell save

   Or maybe the tests weren't good, so you want to remove it:
     $ codeshell trash

   Or you if you know the name of your tests already from the get go you type:
     $ codeshell c.linux my.pretty.test
   Or:
     $ codeshell my.pretty.test c.linux
   as long as 'c.linux' is a known template in the '/home/moisrex/cmd/code-templates' directory.

   If you're using 'trash' instead of 'remove' then you can use trash-cli to manage them:
     $ trash-list
     $ trash-rm       /tmp/...
     $ trash-restore  /tmp/...
     $ trash-put      /tmp/...

```
