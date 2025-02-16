# Github API: Fetch issues with exceeds rate limit prematurely


<div class="ox-hugo-toc toc has-section-numbers">

<div class="heading">Table of Contents</div>

- <span class="section-num">1</span> [Q](#q)
- <span class="section-num">2</span> [A](#a)
    - <span class="section-num">2.1</span> [Basic authentication](#basic-authentication)
    - <span class="section-num">2.2</span> [Using personal access tokens](#using-personal-access-tokens)

</div>
<!--endtoc-->


本文为摘录，原文为： https://stackoverflow.com/questions/33655700/github-api-fetch-issues-with-exceeds-rate-limit-prematurely



## <span class="section-num">1</span> Q {#q}

I am building an app that fetches the issues and pull requests of over 1K github repos, like this.

```text
$ curl -i "https://api.github.com/repos/user/repo/issues?state=closed"
```

My problem is that, after the initial 60 iterations I get a rate limit error:

```json-ts
{
    "message": "API rate limit exceeded for xxx.xxx.xxx.xxx. (But here's the good news: Authenticated requests get a higher rate limit. Check out the documentation for more details.)",
    "documentation_url": "https://developer.github.com/v3/#rate-limiting"
}
```

The document says I can make upto 5000 requests using Authentication Which I registered an oauth for and obtained Client
ID and Client Secret tokens

<https://api.github.com/repos/%7Brepo.name%7D/issues?client_id=...&client_secret>=&#x2026;

Still the rate limit shows up only after about 60 requests.


## <span class="section-num">2</span> A {#a}

The public GitHub API requests are limited to 60 / hour / ip, like you observed. That's why you need authentication.

There are multiple ways to get authenticated when you use the GitHub APIs.


### <span class="section-num">2.1</span> Basic authentication {#basic-authentication}

Basically, you provide the username and the password.

```sh
curl -u your-username "https://api.github.com/repos/user/repo/issues?state=closed"
```

This will prompt you for entering the password.

If you dont want to use the password, you can use a [personal token](https://docs.github.com/en/rest/authentication/authenticating-to-the-rest-api?apiVersion=2022-11-28):

```sh
curl -u username:token "https://api.github.com/repos/user/repo/issues?state=closed"
```


### <span class="section-num">2.2</span> Using personal access tokens {#using-personal-access-tokens}

This is my favorite, but make sure you don't share the token code with others. To generate a new token, open this page,
and you will create the token.

Then you can use it like this:

```sh
curl "https://api.github.com/repos/user/repo/issues?state=closed&access_token=token"
```

(replace the token snippet at the end of the url with your token code)

◎ OAuth

If you want to implement authentication for other users, you should use OAuth. The docs are good in this direction.

