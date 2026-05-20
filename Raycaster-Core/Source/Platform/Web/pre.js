Module.preRun = Module.preRun || [];

Module.preRun.push(() => {
  const persistDir = 'web';

  FS.mkdir(persistDir);
  FS.mount(IDBFS, { autoPersist: true }, persistDir);
  addRunDependency('syncfs');

  FS.syncfs(true, function(err) {
    if (err) console.error(err);

    // ignore self, duplicates, and  directories set up by emscripten
    const ignore = FS.readdir(persistDir).concat([ 'tmp', 'home', 'dev', 'proc', persistDir ]);
    const files = FS.readdir('/').filter(f => !ignore.includes(f));

    // symlink files/dirs in root to persistent directory so reads and writes can happen in the same location
    for(const file of files) {
        FS.symlink("/" + file, persistDir + "/" + file)
    }

    removeRunDependency('syncfs');
  });
});
