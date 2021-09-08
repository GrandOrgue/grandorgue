
import graphene
import requests
import json

with open("./oauth") as oauth:
    token = oauth.read()

headers = {
    "Authorization": "token " + token,
}

with open("./ourorgan-backup/discussion.json") as discussion:
    discussions = json.load(discussion)
    for forum in discussions["forums"]:
        print(forum["name"])

url = 'https://api.github.com/graphql/'
query = """
query {
  repository(owner: "rousseldenis", name: "grandorgue") {
  }
}"""

json = {"query": "{ repositoryOwner(login : \"rousseldenis\") { login repositories (first : 30){ edges { node { name } } } } }"}

r = requests.post(url, json=json, headers=headers)
print(r)
