const t = createThread(() => {

}, ({ err, thread, status }) => {
    print(thread.boot)
})